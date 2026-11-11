#!/usr/bin/env bash
# Подключает JVMTI-агент к запущенному debuggable-приложению.
#
# Использование:
#   ./scripts/attach-agent.sh path/to/libagent.so [опции агента]
#
# Переменные окружения:
#   PKG    пакет приложения (по умолчанию dev.redbyte.vaultnotes)
#   ADB    путь к adb (по умолчанию adb из PATH)
set -euo pipefail

PKG="${PKG:-dev.redbyte.vaultnotes}"
ADB="${ADB:-adb}"

if [[ $# -lt 1 ]]; then
  echo "Использование: $0 path/to/libagent.so [опции агента]" >&2
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

if ! "$ADB" shell pidof "$PKG" >/dev/null; then
  echo "Процесс $PKG не запущен. Сначала откройте приложение." >&2
  exit 1
fi

# Приложение не может загрузить .so из /data/local/tmp, поэтому
# копируем агент в его собственный каталог через run-as.
"$ADB" push "$AGENT_PATH" "$TMP_PATH" >/dev/null
"$ADB" shell "cat $TMP_PATH | run-as $PKG sh -c 'mkdir -p code_cache && cat > code_cache/$AGENT_NAME'"

APP_DIR="$("$ADB" shell run-as "$PKG" pwd | tr -d '\r')"
TARGET="$APP_DIR/code_cache/$AGENT_NAME"

if [[ -n "$OPTIONS" ]]; then
  TARGET="$TARGET=$OPTIONS"
fi

"$ADB" shell cmd activity attach-agent "$PKG" "$TARGET"
echo "Агент $AGENT_NAME подключён к $PKG"
