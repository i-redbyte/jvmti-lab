// RAII для памяти, которую выделяет JVMTI.
//
// Строки и массивы, которые возвращают GetClassSignature, GetMethodName,
// GetLoadedClasses и другие функции, нужно вернуть через Deallocate.
// JvmtiPtr делает это сам в деструкторе — как use {} в Kotlin, только для памяти.
#pragma once

#include <jvmti.h>

#include <utility>

namespace agent {

template <typename T>
class JvmtiPtr {
public:
    explicit JvmtiPtr(jvmtiEnv* jvmti) : jvmti_(jvmti) {}
    ~JvmtiPtr() { reset(); }

    JvmtiPtr(const JvmtiPtr&) = delete;
    JvmtiPtr& operator=(const JvmtiPtr&) = delete;
    JvmtiPtr(JvmtiPtr&& other) noexcept : jvmti_(other.jvmti_), ptr_(std::exchange(other.ptr_, nullptr)) {}

    // Адрес для выходного параметра: jvmti->GetClassSignature(klass, sig.out(), nullptr).
    T** out() {
        reset();
        return &ptr_;
    }

    T* get() const { return ptr_; }
    T& operator[](int i) const { return ptr_[i]; }
    explicit operator bool() const { return ptr_ != nullptr; }

    void reset() {
        if (ptr_ != nullptr) {
            jvmti_->Deallocate(reinterpret_cast<unsigned char*>(ptr_));
            ptr_ = nullptr;
        }
    }

private:
    jvmtiEnv* jvmti_;
    T* ptr_ = nullptr;
};

using JvmtiString = JvmtiPtr<char>;

}  // namespace agent
