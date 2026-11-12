// Строки: UTF-16 → UTF-8, jstring → std::string, маскирование значений для отчёта.
#pragma once

#include <jni.h>

#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace agent {

inline std::string utf16ToUtf8(const jchar* chars, jint length) {
    std::string out;
    out.reserve(static_cast<std::size_t>(length));
    for (jint i = 0; i < length; ++i) {
        unsigned int c = chars[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < length) {   // суррогатная пара
            const unsigned int low = chars[i + 1];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                c = 0x10000 + ((c - 0xD800) << 10) + (low - 0xDC00);
                ++i;
            }
        }
        if (c < 0x80) {
            out += static_cast<char>(c);
        } else if (c < 0x800) {
            out += static_cast<char>(0xC0 | (c >> 6));
            out += static_cast<char>(0x80 | (c & 0x3F));
        } else if (c < 0x10000) {
            out += static_cast<char>(0xE0 | (c >> 12));
            out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (c & 0x3F));
        } else {
            out += static_cast<char>(0xF0 | (c >> 18));
            out += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (c & 0x3F));
        }
    }
    return out;
}

// Содержимое jstring. Звать можно только там, где разрешён JNI (не в колбэках обхода кучи).
inline std::string toStdString(JNIEnv* jni, jstring text) {
    if (text == nullptr) return {};
    const jsize length = jni->GetStringLength(text);
    const jchar* chars = jni->GetStringChars(text, nullptr);
    if (chars == nullptr) return {};
    std::string out = utf16ToUtf8(chars, length);
    jni->ReleaseStringChars(text, chars);
    return out;
}

// Замаскированный вид значения: "eyJh…3Hw (163 симв.)". Полное значение — только с опцией reveal.
inline std::string preview(std::string_view value) {
    char suffix[32];
    std::snprintf(suffix, sizeof(suffix), " (%zu симв.)", value.size());
    if (value.size() <= 8) return std::string(value.size(), '*') + suffix;
    return std::string(value.substr(0, 4)) + "…" + std::string(value.substr(value.size() - 3)) + suffix;
}

inline std::string toHex(const std::vector<jbyte>& bytes) {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (jbyte b : bytes) {
        const auto u = static_cast<unsigned char>(b);
        out += kDigits[u >> 4];
        out += kDigits[u & 0x0F];
    }
    return out;
}

}  // namespace agent
