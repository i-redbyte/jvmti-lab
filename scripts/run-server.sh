#!/usr/bin/env bash
# Запускает локальный сервер VaultNotes на Ktor.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

if [[ ! -x "$ROOT/server/gradlew" ]]; then
  echo "Сервер ещё не готов: он появится вместе с первой статьёй." >&2
  exit 1
fi

cd "$ROOT/server"
exec ./gradlew run
