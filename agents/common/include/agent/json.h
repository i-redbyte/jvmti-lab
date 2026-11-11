// Минимальный JSON для отчётов агента: одна строка — одно событие (формат JSON Lines).
#pragma once

#include <cstdio>
#include <string>
#include <string_view>

namespace agent {

inline std::string jsonEscape(std::string_view text) {
    std::string out;
    out.reserve(text.size() + 2);
    for (char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                    out += buffer;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

// Собирает объект {"key": value, ...} по одному полю.
class JsonObject {
public:
    JsonObject& add(std::string_view key, std::string_view value) {
        begin(key);
        body_ += '"';
        body_ += jsonEscape(value);
        body_ += '"';
        return *this;
    }

    JsonObject& add(std::string_view key, const char* value) {
        return value == nullptr ? addNull(key) : add(key, std::string_view(value));
    }

    JsonObject& add(std::string_view key, long long value) {
        begin(key);
        body_ += std::to_string(value);
        return *this;
    }

    JsonObject& add(std::string_view key, bool value) {
        begin(key);
        body_ += value ? "true" : "false";
        return *this;
    }

    JsonObject& addNull(std::string_view key) {
        begin(key);
        body_ += "null";
        return *this;
    }

    std::string str() const { return "{" + body_ + "}"; }

private:
    void begin(std::string_view key) {
        if (!body_.empty()) body_ += ',';
        body_ += '"';
        body_ += jsonEscape(key);
        body_ += "\":";
    }

    std::string body_;
};

}  // namespace agent
