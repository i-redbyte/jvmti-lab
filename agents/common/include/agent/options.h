// Разбор строки опций агента: "verbose,out=/data/local/tmp/reports".
#pragma once

#include <string>
#include <string_view>

namespace agent {

class Options {
public:
    explicit Options(const char* raw) : raw_(raw == nullptr ? "" : raw) {}

    bool has(std::string_view flag) const { return find(flag).has_value; }

    std::string value(std::string_view key, std::string fallback = {}) const {
        auto entry = find(key);
        return entry.has_value && !entry.value.empty() ? std::string(entry.value) : fallback;
    }

private:
    struct Entry {
        bool has_value = false;
        std::string_view value;
    };

    Entry find(std::string_view key) const {
        std::string_view rest = raw_;
        while (!rest.empty()) {
            auto comma = rest.find(',');
            std::string_view item = rest.substr(0, comma);
            auto eq = item.find('=');
            if (item.substr(0, eq) == key) {
                return {true, eq == std::string_view::npos ? std::string_view{} : item.substr(eq + 1)};
            }
            if (comma == std::string_view::npos) break;
            rest.remove_prefix(comma + 1);
        }
        return {};
    }

    std::string raw_;
};

}  // namespace agent
