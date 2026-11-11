// Raw monitor JVMTI и блокировка в стиле synchronized {}.
//
// Колбэки JVMTI приходят на разных потоках приложения одновременно,
// поэтому общие данные агента (файл отчёта, счётчики) защищаем монитором.
#pragma once

#include <jvmti.h>

namespace agent {

class RawMonitor {
public:
    RawMonitor(jvmtiEnv* jvmti, const char* name) : jvmti_(jvmti) {
        jvmti_->CreateRawMonitor(name, &monitor_);
    }
    ~RawMonitor() { jvmti_->DestroyRawMonitor(monitor_); }

    RawMonitor(const RawMonitor&) = delete;
    RawMonitor& operator=(const RawMonitor&) = delete;

    void lock() { jvmti_->RawMonitorEnter(monitor_); }
    void unlock() { jvmti_->RawMonitorExit(monitor_); }

private:
    jvmtiEnv* jvmti_;
    jrawMonitorID monitor_ = nullptr;
};

// Захватывает монитор в конструкторе и отпускает в деструкторе.
class MonitorLock {
public:
    explicit MonitorLock(RawMonitor& monitor) : monitor_(monitor) { monitor_.lock(); }
    ~MonitorLock() { monitor_.unlock(); }

    MonitorLock(const MonitorLock&) = delete;
    MonitorLock& operator=(const MonitorLock&) = delete;

private:
    RawMonitor& monitor_;
};

}  // namespace agent
