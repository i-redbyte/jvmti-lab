// Файл отчёта агента в каталоге приложения: files/jvmti-reports/<имя>.jsonl.
#pragma once

#include <dlfcn.h>
#include <sys/stat.h>

#include <cstdio>
#include <string>

#include "agent/log.h"

namespace agent {

// Каталог данных приложения, вычисленный по пути к самому агенту.
// Скрипт attach-agent.sh кладёт .so в <data>/code_cache/, значит <data> — всё, что до него.
inline std::string appDataDir() {
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&appDataDir), &info) == 0 || info.dli_fname == nullptr) return {};
    std::string path = info.dli_fname;
    auto pos = path.rfind("/code_cache/");
    return pos == std::string::npos ? std::string{} : path.substr(0, pos);
}

class Report {
public:
    // dir пустой → <data>/files/jvmti-reports.
    bool open(std::string dir, const std::string& name) {
        if (dir.empty()) {
            std::string data = appDataDir();
            if (data.empty()) {
                AGENT_LOGW("Не удалось определить каталог приложения, отчёт не пишется");
                return false;
            }
            mkdir((data + "/files").c_str(), 0700);
            dir = data + "/files/jvmti-reports";
        }
        mkdir(dir.c_str(), 0700);
        path_ = dir + "/" + name + ".jsonl";
        file_ = std::fopen(path_.c_str(), "w");
        if (file_ == nullptr) {
            AGENT_LOGW("Не удалось открыть %s", path_.c_str());
            return false;
        }
        AGENT_LOGI("Отчёт: %s", path_.c_str());
        return true;
    }

    // Вызывать под монитором: запись из разных потоков не должна перемешиваться.
    void write(const std::string& line) {
        if (file_ == nullptr) return;
        std::fputs(line.c_str(), file_);
        std::fputc('\n', file_);
        std::fflush(file_);
    }

    ~Report() {
        if (file_ != nullptr) std::fclose(file_);
    }

private:
    std::string path_;
    std::FILE* file_ = nullptr;
};

}  // namespace agent
