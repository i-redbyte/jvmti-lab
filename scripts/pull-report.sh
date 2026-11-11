#!/usr/bin/env bash
# Забирает JSON-отчёты агента из каталога приложения в out/.
#
# Переменные окружения:
#   PKG    пакет приложения (по умолчанию dev.redbyte.vaultnotes)
#   ADB    путь к adb (по умолчанию adb из PATH)
set -euo pipefail

PKG="${PKG:-dev.redbyte.vaultnotes}"
ADB="${ADB:-adb}"
OUT_DIR="$(cd "$(dirname "$0")/.." && pwd)/out"

mkdir -p "$OUT_DIR"

REPORTS="$("$ADB" shell run-as "$PKG" ls files/jvmti-reports 2>/dev/null | tr -d '\r' || true)"
if [[ -z "$REPORTS" ]]; then
  echo "Отчётов пока нет: files/jvmti-reports в $PKG пуст." >&2
  exit 1
fi

for name in $REPORTS; do
  "$ADB" exec-out run-as "$PKG" cat "files/jvmti-reports/$name" > "$OUT_DIR/$name"
  echo "$OUT_DIR/$name"
done
