// Имена классов и методов в привычном виде.
#pragma once

#include <jvmti.h>

#include <string>
#include <string_view>

#include "agent/jvmti_ptr.h"

namespace agent {

// "Ldev/redbyte/Foo;" → "dev.redbyte.Foo", "[B" остаётся как есть.
inline std::string javaName(std::string_view signature) {
    if (signature.size() >= 2 && signature.front() == 'L' && signature.back() == ';') {
        signature = signature.substr(1, signature.size() - 2);
    }
    std::string name(signature);
    for (char& c : name) {
        if (c == '/') c = '.';
    }
    return name;
}

inline std::string className(jvmtiEnv* jvmti, jclass klass) {
    JvmtiString signature(jvmti);
    if (jvmti->GetClassSignature(klass, signature.out(), nullptr) != JVMTI_ERROR_NONE) return "?";
    return javaName(signature.get());
}

// Имя класса загрузчика: "dalvik.system.PathClassLoader" или "boot" для системных классов.
inline std::string loaderName(jvmtiEnv* jvmti, JNIEnv* jni, jobject loader) {
    if (loader == nullptr) return "boot";
    jclass loaderClass = jni->GetObjectClass(loader);
    std::string name = className(jvmti, loaderClass);
    jni->DeleteLocalRef(loaderClass);
    return name;
}

}  // namespace agent
