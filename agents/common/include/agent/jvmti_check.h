// Проверка кодов jvmtiError с понятным сообщением в logcat.
#pragma once

#include <jvmti.h>

#include "agent/jvmti_ptr.h"
#include "agent/log.h"

namespace agent {

// Возвращает true, если вызов успешен. Иначе пишет в лог имя ошибки, например JVMTI_ERROR_MUST_POSSESS_CAPABILITY.
inline bool check(jvmtiEnv* jvmti, jvmtiError error, const char* what) {
    if (error == JVMTI_ERROR_NONE) return true;
    JvmtiString name(jvmti);
    jvmti->GetErrorName(error, name.out());
    AGENT_LOGE("%s: %s (%d)", what, name ? name.get() : "?", static_cast<int>(error));
    return false;
}

}  // namespace agent
