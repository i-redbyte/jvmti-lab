// Атрибуция по стеку: кто вызвал метод.
#pragma once

#include <jvmti.h>

#include <string>
#include <vector>

#include "agent/jni_ref.h"
#include "agent/jvmti_ptr.h"
#include "agent/names.h"

namespace agent {

inline std::string methodName(jvmtiEnv* jvmti, jmethodID method) {
    JvmtiString name(jvmti);
    if (jvmti->GetMethodName(method, name.out(), nullptr, nullptr) != JVMTI_ERROR_NONE) return "?";
    return name.get();
}

struct Frame {
    std::string className;
    std::string methodName;
    std::string loader;   // "boot" для фреймворка
    jlocation location = 0;
};

// Кадры текущего стека сверху вниз, не больше maxFrames.
inline std::vector<Frame> stackOf(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, jint maxFrames = 32) {
    std::vector<jvmtiFrameInfo> frames(static_cast<std::size_t>(maxFrames));
    jint count = 0;
    std::vector<Frame> out;
    if (jvmti->GetStackTrace(thread, 0, maxFrames, frames.data(), &count) != JVMTI_ERROR_NONE) return out;

    for (jint i = 0; i < count; ++i) {
        jclass declaring = nullptr;
        if (jvmti->GetMethodDeclaringClass(frames[i].method, &declaring) != JVMTI_ERROR_NONE) continue;
        LocalRef<jclass> klass(jni, declaring);
        jobject loader = nullptr;
        jvmti->GetClassLoader(klass.get(), &loader);
        LocalRef<jobject> loaderRef(jni, loader);
        out.push_back({className(jvmti, klass.get()), methodName(jvmti, frames[i].method),
                       loaderName(jvmti, jni, loader), frames[i].location});
    }
    return out;
}

inline bool isFrameworkLoader(const std::string& loader) {
    return loader == "boot" || loader == "java.lang.BootClassLoader";
}

// Первый кадр кода приложения (не boot-загрузчика), пропустив skip верхних кадров.
inline std::string callerOf(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, int skip = 0) {
    const auto frames = stackOf(jvmti, jni, thread);
    for (std::size_t i = static_cast<std::size_t>(skip); i < frames.size(); ++i) {
        if (!isFrameworkLoader(frames[i].loader)) {
            return frames[i].className + "." + frames[i].methodName + " (байткод +" +
                   std::to_string(frames[i].location) + ")";
        }
    }
    return "?";
}

}  // namespace agent
