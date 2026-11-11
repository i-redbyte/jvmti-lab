#!/usr/bin/env bash
# Подключает JVMTI-агент к debuggable-приложению.
#
# Использование:
#   ./scripts/attach-agent.sh [--start] path/to/libagent.so [опции агента]
#
#   без --start  агент подключается к уже запущенному процессу (cmd activity attach-agent);
#   --start      приложение перезапускается, и агент подключается при старте (am start --attach-agent),
#                так он видит и то, что происходит до первого экрана.
#
# Переменные окружения:
#   PKG             пакет приложения (по умолчанию dev.redbyte.vaultnotes)
#   ACTIVITY        активность для --start (по умолчанию .ui.MainActivity)
#   ANDROID_SERIAL  устройство, если их подключено несколько (это переменная самого adb)
set -euo pipefail

PKG="${PKG:-dev.redbyte.vaultnotes}"
ACTIVITY="${ACTIVITY:-.ui.MainActivity}"
ADB="${ADB:-adb}"

START=0
if [[ "${1:-}" == "--start" ]]; then
  START=1
  shift
fi

if [[ $# -lt 1 ]]; then
  echo "Использование: $0 [--start] path/to/libagent.so [опции агента]" >&2
  exit 1
fi

AGENT_PATH="$1"
OPTIONS="${2:-}"
AGENT_NAME="$(basename "$AGENT_PATH")"
TMP_PATH="/data/local/tmp/$AGENT_NAME"

if [[ ! -f "$AGENT_PATH" ]]; then
  echo "Агент не найден: $AGENT_PATH" >&2
  exit 1
fi

# Приложение не может загрузить .so из /data/local/tmp, поэтому копируем агент
# в его собственный каталог через run-as. run-as работает только для debuggable-приложений.
"$ADB" push "$AGENT_PATH" "$TMP_PATH" >/dev/null
"$ADB" shell "cat $TMP_PATH | run-as $PKG sh -c 'mkdir -p code_cache && cat > code_cache/$AGENT_NAME'"
"$ADB" shell rm -f "$TMP_PATH"

APP_DIR="$("$ADB" shell run-as "$PKG" pwd | tr -d '\r')"
TARGET="$APP_DIR/code_cache/$AGENT_NAME"
if [[ -n "$OPTIONS" ]]; then
  TARGET="$TARGET=$OPTIONS"
fi

if [[ $START -eq 1 ]]; then
  "$ADB" shell am force-stop "$PKG"
  "$ADB" shell am start -n "$PKG/$ACTIVITY" --attach-agent "$TARGET" >/dev/null
  echo "$PKG запущен с агентом $AGENT_NAME"
else
  if ! "$ADB" shell pidof "$PKG" >/dev/null; then
    echo "Процесс $PKG не запущен. Откройте приложение или используйте --start." >&2
    exit 1
  fi
  "$ADB" shell cmd activity attach-agent "$PKG" "$TARGET"
  echo "Агент $AGENT_NAME подключён к $PKG"
fi

echo "Лог агента: adb logcat -s jvmti-recon (тег зависит от агента)"
