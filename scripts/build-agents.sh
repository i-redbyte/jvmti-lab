#!/usr/bin/env bash
# Собирает все агенты для нужной архитектуры.
#
# Использование:
#   ./scripts/build-agents.sh [abi]
#
# abi по умолчанию берётся с подключённого устройства (adb), иначе arm64-v8a.
# Результат: agents/build/<abi>/part1-recon/libagent_recon.so и т. д.
#
# Переменные окружения:
#   ANDROID_NDK   путь к NDK (по умолчанию $ANDROID_HOME/ndk/30.0.16248370)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SDK="${ANDROID_HOME:-${ANDROID_SDK_ROOT:-$HOME/Library/Android/sdk}}"
NDK="${ANDROID_NDK:-$SDK/ndk/30.0.16248370}"

ABI="${1:-}"
if [[ -z "$ABI" ]]; then
  ABI="$(adb shell getprop ro.product.cpu.abi 2>/dev/null | tr -d '\r' || true)"
  ABI="${ABI:-arm64-v8a}"
fi

BUILD_DIR="$ROOT/agents/build/$ABI"

cmake -S "$ROOT/agents" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_PLATFORM=android-28 \
  -DANDROID_STL=c++_static \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build "$BUILD_DIR"

echo
find "$BUILD_DIR" -name 'libagent_*.so' -print
