// Имена полей по номеру из FollowReferences.
//
// Колбэк FollowReferences даёт только номер поля (reference_info->field.index), а звать
// GetClassFields внутри колбэка нельзя. Поэтому номер переводим в имя уже после обхода.
//
// Как JVMTI считает номер (спецификация, раздел Heap Reference Information - Field):
// номера идут по общему списку полей класса и всех его предков. Сначала поля суперкласса
// (рекурсивно, от java.lang.Object вниз), затем поля интерфейсов, которые класс реализует
// напрямую, затем собственные поля класса в порядке GetClassFields. Пример: у
// `class Session(user, token, secret) : Any()` предков с полями нет, поэтому
// user = 0, token = 1, secret = 2. Статические и нестатические поля нумеруются вместе.
#pragma once

#include <jvmti.h>

#include <string>
#include <vector>

#include "agent/jvmti_ptr.h"

namespace agent {

struct FieldName {
    std::string name;
    std::string signature;
    bool isStatic = false;
};

inline void appendDeclaredFields(jvmtiEnv* jvmti, jclass klass, std::vector<FieldName>& out) {
    jint count = 0;
    JvmtiPtr<jfieldID> fields(jvmti);
    if (jvmti->GetClassFields(klass, &count, fields.out()) != JVMTI_ERROR_NONE) return;
    for (jint i = 0; i < count; ++i) {
        JvmtiString name(jvmti);
        JvmtiString signature(jvmti);
        jvmti->GetFieldName(klass, fields[i], name.out(), signature.out(), nullptr);
        jint modifiers = 0;
        jvmti->GetFieldModifiers(klass, fields[i], &modifiers);
        out.push_back({name ? name.get() : "?", signature ? signature.get() : "?", (modifiers & 0x0008) != 0});
    }
}

// Общий список полей в той нумерации, которую использует FollowReferences.
inline std::vector<FieldName> indexedFields(jvmtiEnv* jvmti, JNIEnv* jni, jclass klass) {
    std::vector<FieldName> out;
    if (klass == nullptr) return out;

    jclass super = jni->GetSuperclass(klass);
    if (super != nullptr) {
        out = indexedFields(jvmti, jni, super);
        jni->DeleteLocalRef(super);
    }

    jint interfaceCount = 0;
    JvmtiPtr<jclass> interfaces(jvmti);
    if (jvmti->GetImplementedInterfaces(klass, &interfaceCount, interfaces.out()) == JVMTI_ERROR_NONE) {
        for (jint i = 0; i < interfaceCount; ++i) {
            appendDeclaredFields(jvmti, interfaces[i], out);
            jni->DeleteLocalRef(interfaces[i]);
        }
    }

    appendDeclaredFields(jvmti, klass, out);
    return out;
}

}  // namespace agent
