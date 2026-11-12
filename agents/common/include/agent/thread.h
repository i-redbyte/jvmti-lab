// JNIEnv для текущего потока.
//
// JNIEnv привязан к потоку. Если код агента оказался на потоке, который виртуальная машина
// не знает (например, на своём std::thread), к ней надо прицепиться и потом отцепиться.
#pragma once

#include <jni.h>

namespace agent {

class ScopedJniEnv {
public:
    explicit ScopedJniEnv(JavaVM* vm, const char* threadName = "jvmti-agent") : vm_(vm) {
        if (vm_->GetEnv(reinterpret_cast<void**>(&env_), JNI_VERSION_1_6) == JNI_EDETACHED) {
            JavaVMAttachArgs args{JNI_VERSION_1_6, threadName, nullptr};
            if (vm_->AttachCurrentThread(&env_, &args) == JNI_OK) {
                attached_ = true;   // прицепились сами, значит, сами и отцепимся
            } else {
                env_ = nullptr;
            }
        }
    }

    ~ScopedJniEnv() {
        if (attached_) vm_->DetachCurrentThread();
    }

    ScopedJniEnv(const ScopedJniEnv&) = delete;
    ScopedJniEnv& operator=(const ScopedJniEnv&) = delete;

    JNIEnv* get() const { return env_; }

private:
    JavaVM* vm_;
    JNIEnv* env_ = nullptr;
    bool attached_ = false;
};

}  // namespace agent
