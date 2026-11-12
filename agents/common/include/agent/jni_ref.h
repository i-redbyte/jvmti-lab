// RAII для ссылок JNI.
//
// LocalRef сам зовёт DeleteLocalRef в деструкторе, GlobalRef — DeleteGlobalRef.
// Так ссылки возвращаются, даже если функция вышла раньше по ошибке.
#pragma once

#include <jni.h>

#include <utility>

namespace agent {

template <typename T>
class LocalRef {
public:
    LocalRef(JNIEnv* jni, T ref) : jni_(jni), ref_(ref) {}
    ~LocalRef() {
        if (ref_ != nullptr) jni_->DeleteLocalRef(ref_);
    }

    LocalRef(const LocalRef&) = delete;
    LocalRef& operator=(const LocalRef&) = delete;
    LocalRef(LocalRef&& other) noexcept : jni_(other.jni_), ref_(std::exchange(other.ref_, nullptr)) {}

    T get() const { return ref_; }
    explicit operator bool() const { return ref_ != nullptr; }

private:
    JNIEnv* jni_;
    T ref_;
};

class GlobalRef {
public:
    GlobalRef() = default;
    GlobalRef(JNIEnv* jni, jobject ref) : ref_(ref != nullptr ? jni->NewGlobalRef(ref) : nullptr) {}

    GlobalRef(const GlobalRef&) = delete;
    GlobalRef& operator=(const GlobalRef&) = delete;

    // Глобальная ссылка живёт, пока её явно не отпустили: в деструкторе нет JNIEnv,
    // поэтому отпускаем через release(jni).
    void release(JNIEnv* jni) {
        if (ref_ != nullptr) jni->DeleteGlobalRef(ref_);
        ref_ = nullptr;
    }

    jobject get() const { return ref_; }

private:
    jobject ref_ = nullptr;
};

}  // namespace agent
