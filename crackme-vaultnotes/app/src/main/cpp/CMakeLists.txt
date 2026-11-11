# libvault.so: проверка PIN и ключ премиум-модуля.
cmake_minimum_required(VERSION 3.22)
project(vault CXX)

add_library(vault SHARED vault.cpp)

target_compile_features(vault PRIVATE cxx_std_20)
# Наружу смотрят только JNI_OnLoad и функции с JNIEXPORT, остальное скрыто.
target_compile_options(vault PRIVATE -O2 -Wall -Wextra -fvisibility=hidden -fvisibility-inlines-hidden)
# Ключ приходит из gradle.properties через аргумент CMake.
target_compile_definitions(vault PRIVATE VAULT_PREMIUM_KEY_HEX="${VAULT_PREMIUM_KEY_HEX}")
# Страницы по 16 КБ: новые устройства требуют такого выравнивания сегментов .so.
target_link_options(vault PRIVATE "-Wl,-z,max-page-size=16384")
target_link_libraries(vault log)
