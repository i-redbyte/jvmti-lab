// Слоты аргументов метода в ART.
//
// В HotSpot аргументы занимают первые слоты кадра. В ART слоты — это регистры dex-байткода,
// а аргументы лежат в ПОСЛЕДНИХ регистрах кадра. Поэтому первый слот аргументов равен
// «всего регистров» (GetMaxLocals) минус «регистров под аргументы» (GetArgumentsSize).
#pragma once

#include <jvmti.h>

#include <string_view>
#include <vector>

namespace agent {

// Типы аргументов из JNI-сигнатуры: "(Ljava/lang/String;I[B)V" → {'L', 'I', '['}.
inline std::vector<char> parseArgs(std::string_view signature) {
    std::vector<char> kinds;
    std::size_t i = signature.find('(');
    if (i == std::string_view::npos) return kinds;
    ++i;
    while (i < signature.size() && signature[i] != ')') {
        const char start = signature[i];
        while (i < signature.size() && signature[i] == '[') ++i;   // массив любой размерности
        if (i < signature.size() && signature[i] == 'L') {
            i = signature.find(';', i);
            if (i == std::string_view::npos) break;
        }
        kinds.push_back(start);   // '[' для массивов, 'L' для объектов, иначе примитив
        ++i;
    }
    return kinds;
}

// Слот аргумента статического метода с простыми аргументами (без long/double до него).
inline jint argSlot(jvmtiEnv* jvmti, jmethodID method, int argIndex) {
    jint maxLocals = 0;
    jint argsSize = 0;
    jvmti->GetMaxLocals(method, &maxLocals);
    jvmti->GetArgumentsSize(method, &argsSize);
    return maxLocals - argsSize + argIndex;
}

struct ArgLayout {
    std::vector<jint> slots;   // slots[i] — слот i-го объявленного аргумента
    std::vector<char> kinds;   // 'L' объект, '[' массив, 'I' int, 'J' long, 'D' double, ...
    jint thisSlot = -1;        // слот this у нестатического метода
};

// Точная раскладка аргументов любого метода: с this, long и double.
inline ArgLayout argLayout(jvmtiEnv* jvmti, jmethodID method, std::string_view signature) {
    jint maxLocals = 0;
    jint argsSize = 0;   // размер в слотах, включая this у нестатических
    jvmti->GetMaxLocals(method, &maxLocals);
    jvmti->GetArgumentsSize(method, &argsSize);

    ArgLayout layout;
    jint slot = maxLocals - argsSize;   // первый слот под аргументы

    jint modifiers = 0;
    jvmti->GetMethodModifiers(method, &modifiers);
    const bool isStatic = (modifiers & 0x0008) != 0;   // ACC_STATIC
    if (!isStatic) {
        layout.thisSlot = slot;
        slot += 1;
    }

    for (char kind : parseArgs(signature)) {
        layout.slots.push_back(slot);
        layout.kinds.push_back(kind);
        slot += (kind == 'J' || kind == 'D') ? 2 : 1;   // long и double занимают по два слота
    }
    return layout;
}

}  // namespace agent
