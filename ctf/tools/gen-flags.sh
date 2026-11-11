#!/usr/bin/env bash
# Генерирует случайные флаги VaultNotes в ctf/secrets/ (каталог не попадает в git).
#
#   ctf/secrets/flags.properties   id=vault{...}   — их Gradle подставляет в приложение
#   ctf/secrets/flags.sha256       id хэш          — по ним check-flag.sh проверяет вашу сборку
#
# Использование:
#   ./ctf/tools/gen-flags.sh            создать флаги, если их ещё нет
#   ./ctf/tools/gen-flags.sh --force    создать новые флаги (старые сборки перестанут сходиться)
#   ./ctf/tools/gen-flags.sh --publish  скопировать хэши в ctf/flags.sha256 (для официального APK)
set -euo pipefail

CTF="$(cd "$(dirname "$0")/.." && pwd)"
SECRETS="$CTF/secrets"
FLAGS="$SECRETS/flags.properties"
HASHES="$SECRETS/flags.sha256"

sha256() {
  if command -v sha256sum >/dev/null; then sha256sum | cut -d' ' -f1; else shasum -a 256 | cut -d' ' -f1; fi
}

if [[ "${1:-}" == "--publish" ]]; then
  [[ -f "$HASHES" ]] || { echo "Нет $HASHES: сначала создайте флаги." >&2; exit 1; }
  cp "$HASHES" "$CTF/flags.sha256"
  echo "Хэши скопированы в ctf/flags.sha256"
  exit 0
fi

if [[ -f "$FLAGS" && "${1:-}" != "--force" ]]; then
  echo "Флаги уже есть: $FLAGS (новые: --force)"
  exit 0
fi

mkdir -p "$SECRETS"
chmod 700 "$SECRETS"
: > "$FLAGS"
: > "$HASHES"
chmod 600 "$FLAGS"

# Список заданий берём из ctf/tasks.tsv (первая колонка, без заголовка).
tail -n +2 "$CTF/tasks.tsv" | cut -f1 | while read -r id; do
  flag="vault{$(openssl rand -hex 12)}"
  echo "$id=$flag" >> "$FLAGS"
  echo "$id $(printf '%s' "$flag" | sha256)" >> "$HASHES"
done

echo "Флаги: $FLAGS"
echo "Хэши:  $HASHES"
echo "Сохраните копию ctf/secrets/: без неё хэши официального APK не восстановить."
