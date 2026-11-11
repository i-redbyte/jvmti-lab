# Все агенты цикла. Собираются отдельно от приложения: VaultNotes о них ничего не знает.
# Сборка: ./scripts/build-agents.sh (или cmake с android.toolchain.cmake из NDK).
cmake_minimum_required(VERSION 3.22)
project(jvmti_agents CXX)

# Общие флаги для всех агентов.
function(jvmti_agent target)
    add_library(${target} SHARED ${ARGN})
    target_link_libraries(${target} PRIVATE agent_common)
    # Наружу смотрят только Agent_OnAttach и Agent_OnLoad.
    target_compile_options(${target} PRIVATE -O2 -Wall -Wextra -fvisibility=hidden -fvisibility-inlines-hidden -fno-exceptions)
    # Страницы по 16 КБ: новые устройства требуют такого выравнивания сегментов .so.
    # --exclude-libs: статическая libc++ не торчит наружу символами из нашей .so.
    target_link_options(${target} PRIVATE "-Wl,-z,max-page-size=16384" "-Wl,--exclude-libs,ALL")
endfunction()

add_subdirectory(common)
add_subdirectory(part1-recon)
