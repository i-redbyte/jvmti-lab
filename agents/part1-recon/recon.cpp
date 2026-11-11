// part1-recon — первый агент цикла: разведка приложения.
//
// Что делает:
//   1. Печатает, какие возможности (capabilities) JVMTI есть у ART на этом устройстве.
//   2. Перечисляет уже загруженные классы приложения и следит за новыми (ClassPrepare):
//      какой загрузчик принёс каждый класс. Классы не из APK сразу видно.
//   3. Ловит связывание native-методов (NativeMethodBind) и через dladdr показывает,
//      в какой библиотеке и под каким символом лежит их код.
//
// Вывод: logcat с тегом jvmti-recon и отчёт files/jvmti-reports/recon.jsonl.
// Опции: verbose — печатать в logcat каждый класс приложения; out=<каталог> — куда писать отчёт.

#include <dlfcn.h>
#include <jvmti.h>
#include <unistd.h>

#include <map>
#include <memory>
#include <string>

#include "agent/json.h"
#include "agent/jvmti_check.h"
#include "agent/jvmti_ptr.h"
#include "agent/log.h"
#include "agent/names.h"
#include "agent/options.h"
#include "agent/raw_monitor.h"
#include "agent/report.h"

namespace {

using agent::JsonObject;
using agent::JvmtiString;
using agent::MonitorLock;

constexpr const char* kAppLoader = "dalvik.system.PathClassLoader";
constexpr const char* kBootLoader = "java.lang.BootClassLoader";

// Всё состояние агента. Живёт до конца процесса: выгрузить агент из ART нельзя.
struct Recon {
    Recon(jvmtiEnv* env, bool verboseLog) : jvmti(env), lock(env, "recon-lock"), verbose(verboseLog) {}

    jvmtiEnv* jvmti;
    agent::RawMonitor lock;
    agent::Report report;
    bool verbose;
    std::map<std::string, int> classesPerLoader;
};

Recon* gRecon = nullptr;

// ---------- Возможности ----------

void reportCapabilities(jvmtiEnv* jvmti) {
    jvmtiCapabilities caps{};
    if (!agent::check(jvmti, jvmti->GetPotentialCapabilities(&caps), "GetPotentialCapabilities")) return;

    JsonObject json;
    json.add("event", "capabilities");
    // Те возможности, на которых держатся агенты цикла.
    auto add = [&](const char* name, unsigned int value) {
        json.add(name, value != 0);
        AGENT_LOGI("  %-42s %s", name, value ? "да" : "нет");
    };
    AGENT_LOGI("Возможности JVMTI в этом ART:");
    add("can_tag_objects", caps.can_tag_objects);
    add("can_generate_field_modification_events", caps.can_generate_field_modification_events);
    add("can_generate_field_access_events", caps.can_generate_field_access_events);
    add("can_get_bytecodes", caps.can_get_bytecodes);
    add("can_get_owned_monitor_info", caps.can_get_owned_monitor_info);
    add("can_pop_frame", caps.can_pop_frame);
    add("can_redefine_classes", caps.can_redefine_classes);
    add("can_signal_thread", caps.can_signal_thread);
    add("can_get_source_file_name", caps.can_get_source_file_name);
    add("can_get_line_numbers", caps.can_get_line_numbers);
    add("can_access_local_variables", caps.can_access_local_variables);
    add("can_generate_single_step_events", caps.can_generate_single_step_events);
    add("can_generate_exception_events", caps.can_generate_exception_events);
    add("can_generate_frame_pop_events", caps.can_generate_frame_pop_events);
    add("can_generate_breakpoint_events", caps.can_generate_breakpoint_events);
    add("can_suspend", caps.can_suspend);
    add("can_get_current_thread_cpu_time", caps.can_get_current_thread_cpu_time);
    add("can_generate_method_entry_events", caps.can_generate_method_entry_events);
    add("can_generate_method_exit_events", caps.can_generate_method_exit_events);
    add("can_generate_all_class_hook_events", caps.can_generate_all_class_hook_events);
    add("can_generate_compiled_method_load_events", caps.can_generate_compiled_method_load_events);
    add("can_generate_monitor_events", caps.can_generate_monitor_events);
    add("can_generate_vm_object_alloc_events", caps.can_generate_vm_object_alloc_events);
    add("can_generate_native_method_bind_events", caps.can_generate_native_method_bind_events);
    add("can_generate_garbage_collection_events", caps.can_generate_garbage_collection_events);
    add("can_generate_object_free_events", caps.can_generate_object_free_events);
    add("can_force_early_return", caps.can_force_early_return);
    add("can_get_constant_pool", caps.can_get_constant_pool);
    add("can_set_native_method_prefix", caps.can_set_native_method_prefix);
    add("can_retransform_classes", caps.can_retransform_classes);
    add("can_retransform_any_class", caps.can_retransform_any_class);
    add("can_generate_resource_exhaustion_heap_events", caps.can_generate_resource_exhaustion_heap_events);

    MonitorLock guard(gRecon->lock);
    gRecon->report.write(json.str());
}

// ---------- Классы ----------

void describeClass(jvmtiEnv* jvmti, JNIEnv* jni, jclass klass, const char* when) {
    jobject loader = nullptr;
    if (jvmti->GetClassLoader(klass, &loader) != JVMTI_ERROR_NONE) return;
    // Загрузчик null — системный (boot) класс фреймворка. Нас интересует только код приложения.
    if (loader == nullptr) return;

    const std::string loaderClass = agent::loaderName(jvmti, jni, loader);
    jint loaderId = 0;
    jvmti->GetObjectHashCode(loader, &loaderId);
    jni->DeleteLocalRef(loader);
    // BootClassLoader — Java-обёртка над тем же системным загрузчиком (так грузятся, например, $Proxy фреймворка).
    if (loaderClass == kBootLoader) return;

    const std::string name = agent::className(jvmti, klass);

    const bool fromApk = loaderClass == kAppLoader;

    MonitorLock guard(gRecon->lock);
    gRecon->classesPerLoader[loaderClass]++;
    gRecon->report.write(JsonObject()
                             .add("event", "class")
                             .add("when", when)
                             .add("class", name)
                             .add("loader", loaderClass)
                             .add("loaderId", static_cast<long long>(loaderId))
                             .add("fromApk", fromApk)
                             .str());
    if (!fromApk) {
        AGENT_LOGW("Класс не из APK: %s ← %s@%08x", name.c_str(), loaderClass.c_str(), loaderId);
    } else if (gRecon->verbose) {
        AGENT_LOGI("Класс: %s", name.c_str());
    }
}

void JNICALL onClassPrepare(jvmtiEnv* jvmti, JNIEnv* jni, jthread /* thread */, jclass klass) {
    describeClass(jvmti, jni, klass, "prepare");
}

// Классы, загруженные до подключения агента: событий о них уже не будет, поэтому спрашиваем список.
void scanLoadedClasses(jvmtiEnv* jvmti, JNIEnv* jni) {
    jint count = 0;
    agent::JvmtiPtr<jclass> classes(jvmti);
    if (!agent::check(jvmti, jvmti->GetLoadedClasses(&count, classes.out()), "GetLoadedClasses")) return;

    for (jint i = 0; i < count; ++i) {
        describeClass(jvmti, jni, classes[i], "loaded");
        jni->DeleteLocalRef(classes[i]);
    }

    MonitorLock guard(gRecon->lock);
    AGENT_LOGI("Всего загружено классов: %d. Классы приложения по загрузчикам:", count);
    for (const auto& [loader, n] : gRecon->classesPerLoader) {
        AGENT_LOGI("  %-45s %d", loader.c_str(), n);
    }
}

// ---------- Native-методы ----------

void JNICALL onNativeMethodBind(jvmtiEnv* jvmti, JNIEnv* jni, jthread /* thread */, jmethodID method,
                                void* address, void** /* new_address_ptr */) {
    jclass declaring = nullptr;
    if (jvmti->GetMethodDeclaringClass(method, &declaring) != JVMTI_ERROR_NONE) return;

    jobject loader = nullptr;
    jvmti->GetClassLoader(declaring, &loader);
    if (loader == nullptr) {
        // Native-методы фреймворка связываются постоянно, они нам не интересны.
        if (jni != nullptr) jni->DeleteLocalRef(declaring);
        return;
    }

    const std::string owner = agent::className(jvmti, declaring);
    JvmtiString name(jvmti);
    JvmtiString signature(jvmti);
    jvmti->GetMethodName(method, name.out(), signature.out(), nullptr);
    if (jni != nullptr) {
        jni->DeleteLocalRef(loader);
        jni->DeleteLocalRef(declaring);
    }

    // dladdr по адресу кода отвечает, в какой .so он лежит и какой у него символ.
    // Символа нет, если функция скрыта и зарегистрирована через RegisterNatives.
    Dl_info info{};
    const bool found = dladdr(address, &info) != 0;
    const char* library = found ? info.dli_fname : nullptr;
    const char* symbol = found ? info.dli_sname : nullptr;
    const long long offset =
        found ? static_cast<long long>(static_cast<char*>(address) - static_cast<char*>(info.dli_fbase)) : 0;

    MonitorLock guard(gRecon->lock);
    gRecon->report.write(JsonObject()
                             .add("event", "native-bind")
                             .add("class", owner)
                             .add("method", name.get())
                             .add("signature", signature.get())
                             .add("library", library)
                             .add("symbol", symbol)
                             .add("offset", offset)
                             .str());
    AGENT_LOGW("native %s.%s%s → %s +0x%llx (%s)", owner.c_str(), name.get(), signature.get(),
               library ? library : "?", offset, symbol ? symbol : "символ скрыт");
}

// ---------- Запуск ----------

jint start(JavaVM* vm, char* rawOptions, bool attached) {
    agent::logTag() = "jvmti-recon";
    const agent::Options options(rawOptions);

    jvmtiEnv* jvmti = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&jvmti), JVMTI_VERSION_1_2) != JNI_OK) {
        AGENT_LOGE("GetEnv(JVMTI_VERSION_1_2) не удался: приложение не debuggable?");
        return JNI_ERR;
    }

    gRecon = new Recon(jvmti, options.has("verbose"));
    gRecon->report.open(options.value("out"), "recon");
    AGENT_LOGI("Агент подключён к процессу %d (%s)", getpid(), attached ? "attach" : "onload");
    gRecon->report.write(JsonObject().add("event", "agent").add("pid", static_cast<long long>(getpid())).str());

    reportCapabilities(jvmti);

    jvmtiCapabilities wanted{};
    wanted.can_generate_native_method_bind_events = 1;
    if (!agent::check(jvmti, jvmti->AddCapabilities(&wanted), "AddCapabilities")) return JNI_ERR;

    jvmtiEventCallbacks callbacks{};
    callbacks.ClassPrepare = onClassPrepare;
    callbacks.NativeMethodBind = onNativeMethodBind;
    if (!agent::check(jvmti, jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks)), "SetEventCallbacks")) {
        return JNI_ERR;
    }
    agent::check(jvmti, jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, nullptr),
                 "enable ClassPrepare");
    agent::check(jvmti, jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, nullptr),
                 "enable NativeMethodBind");

    if (attached) {
        JNIEnv* jni = nullptr;
        if (vm->GetEnv(reinterpret_cast<void**>(&jni), JNI_VERSION_1_6) == JNI_OK) {
            scanLoadedClasses(jvmti, jni);
        }
    }
    return JNI_OK;
}

}  // namespace

// Подключение к уже запущенному процессу: cmd activity attach-agent или am start --attach-agent.
extern "C" JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM* vm, char* options, void* /* reserved */) {
    return start(vm, options, true);
}

// Подключение при старте виртуальной машины (-agentpath). На Android встречается редко.
extern "C" JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* /* reserved */) {
    return start(vm, options, false);
}
