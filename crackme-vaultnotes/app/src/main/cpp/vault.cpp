// libvault.so — нативная часть VaultNotes.
//
// Автор приложения считает, что секреты в C++ спрятаны надёжнее, чем в Kotlin:
// они хранятся «замаскированными» и собираются только во время вызова.

#include <jni.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace {

constexpr std::uint8_t kMask = 0x5A;

// Строки маскируются ещё при компиляции, поэтому в .so их не найти через strings.
template <std::size_t N>
struct Masked {
    std::array<std::uint8_t, N> bytes{};
};

constexpr std::uint8_t hexDigit(char c) {
    if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<std::uint8_t>(c - 'a' + 10);
    return static_cast<std::uint8_t>(c - 'A' + 10);
}

template <std::size_t N>
constexpr Masked<N> maskHex(std::string_view hex) {
    Masked<N> result;
    for (std::size_t i = 0; i < N; ++i) {
        auto byte = static_cast<std::uint8_t>(hexDigit(hex[2 * i]) << 4 | hexDigit(hex[2 * i + 1]));
        result.bytes[i] = byte ^ static_cast<std::uint8_t>(kMask + i);
    }
    return result;
}

template <std::size_t N>
constexpr Masked<N - 1> maskText(const char (&text)[N]) {
    Masked<N - 1> result;
    for (std::size_t i = 0; i + 1 < N; ++i) {
        result.bytes[i] = static_cast<std::uint8_t>(text[i]) ^ static_cast<std::uint8_t>(kMask + i);
    }
    return result;
}

constexpr auto kPremiumKey = maskHex<16>(VAULT_PREMIUM_KEY_HEX);
constexpr auto kPin = maskText("4821");
constexpr auto kSyncToken = maskText(VAULT_SYNC_TOKEN);

// Отладочный флаг для тестов синхронизации: vault{68b9823f0c3cda78151f6304}
// TODO удалить перед релизом.
__attribute__((used, retain)) const char kDebugSyncToken[] = "vault{d08084a837df11d60461822f}";

template <std::size_t N>
std::uint8_t unmask(const Masked<N>& masked, std::size_t i) {
    return masked.bytes[i] ^ static_cast<std::uint8_t>(kMask + i);
}

// Регистрируется через RegisterNatives: символа с именем Java_... в .so нет.
jboolean checkPin(JNIEnv* env, jobject /* this */, jstring pin) {
    if (pin == nullptr) return JNI_FALSE;
    const jsize length = env->GetStringUTFLength(pin);
    if (static_cast<std::size_t>(length) != kPin.bytes.size()) return JNI_FALSE;

    const char* chars = env->GetStringUTFChars(pin, nullptr);
    bool same = true;
    for (std::size_t i = 0; i < kPin.bytes.size(); ++i) {
        same &= static_cast<std::uint8_t>(chars[i]) == unmask(kPin, i);
    }
    env->ReleaseStringUTFChars(pin, chars);
    return same ? JNI_TRUE : JNI_FALSE;
}

// Токен синхронизации заметок. Регистрируется через RegisterNatives, как и checkPin.
jstring syncToken(JNIEnv* env, jobject /* this */) {
    char token[kSyncToken.bytes.size() + 1]{};
    for (std::size_t i = 0; i < kSyncToken.bytes.size(); ++i) {
        token[i] = static_cast<char>(unmask(kSyncToken, i));
    }
    return env->NewStringUTF(token);
}

const JNINativeMethod kMethods[] = {
    {"checkPin", "(Ljava/lang/String;)Z", reinterpret_cast<void*>(checkPin)},
    {"syncToken", "()Ljava/lang/String;", reinterpret_cast<void*>(syncToken)},
};

}  // namespace

// Связывается по имени: ART сам найдёт этот символ при первом вызове.
extern "C" JNIEXPORT jbyteArray JNICALL
Java_dev_redbyte_vaultnotes_vault_VaultNative_premiumKey(JNIEnv* env, jobject /* this */) {
    std::array<jbyte, kPremiumKey.bytes.size()> key{};
    for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<jbyte>(unmask(kPremiumKey, i));
    }
    jbyteArray result = env->NewByteArray(static_cast<jsize>(key.size()));
    env->SetByteArrayRegion(result, 0, static_cast<jsize>(key.size()), key.data());
    return result;
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;

    jclass vaultClass = env->FindClass("dev/redbyte/vaultnotes/vault/VaultNative");
    if (vaultClass == nullptr) return JNI_ERR;
    if (env->RegisterNatives(vaultClass, kMethods, std::size(kMethods)) != JNI_OK) return JNI_ERR;
    env->DeleteLocalRef(vaultClass);
    return JNI_VERSION_1_6;
}
