// Логирование агента в logcat.
#pragma once

#include <android/log.h>

#include <cstdarg>

namespace agent {

// Тег, под которым агент пишет в logcat. Каждый агент задаёт свой в начале Agent_OnAttach.
inline const char*& logTag() {
    static const char* tag = "jvmti-agent";
    return tag;
}

// format(printf, ...) просит компилятор проверять аргументы, как у printf.
__attribute__((format(printf, 2, 3))) inline void log(int priority, const char* format, ...) {
    va_list args;
    va_start(args, format);
    __android_log_vprint(priority, logTag(), format, args);
    va_end(args);
}

}  // namespace agent

#define AGENT_LOGI(...) ::agent::log(ANDROID_LOG_INFO, __VA_ARGS__)
#define AGENT_LOGW(...) ::agent::log(ANDROID_LOG_WARN, __VA_ARGS__)
#define AGENT_LOGE(...) ::agent::log(ANDROID_LOG_ERROR, __VA_ARGS__)
