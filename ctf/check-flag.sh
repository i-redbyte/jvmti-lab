#!/usr/bin/env bash
# Проверяет найденный флаг.
#
# Использование:
#   ./ctf/check-flag.sh 'vault{...}'
#   ./ctf/check-flag.sh               (спросит флаг)
#
# Флаги официального APK сверяются с ctf/flags.sha256.
# Если вы собрали VaultNotes сами, ваши флаги сверяются с ctf/secrets/flags.sha256.
set -euo pipefail

CTF="$(cd "$(dirname "$0")" && pwd)"

FLAG="${1:-}"
if [[ -z "$FLAG" ]]; then
  read -r -p "Флаг: " FLAG
fi
FLAG="$(printf '%s' "$FLAG" | tr -d '[:space:]')"

if [[ ! "$FLAG" =~ ^vault\{[^}]+\}$ ]]; then
  echo "Флаг выглядит так: vault{...}"
  exit 1
fi

if command -v sha256sum >/dev/null; then
  HASH="$(printf '%s' "$FLAG" | sha256sum | cut -d' ' -f1)"
else
  HASH="$(printf '%s' "$FLAG" | shasum -a 256 | cut -d' ' -f1)"
fi

for file in "$CTF/secrets/flags.sha256" "$CTF/flags.sha256"; do
  [[ -f "$file" ]] || continue
  ID="$(awk -v h="$HASH" '$2 == h { print $1 }' "$file")"
  if [[ -n "$ID" ]]; then
    TITLE="$(head -n 1 "$CTF/tasks/$ID.md" 2>/dev/null | sed 's/^# //')"
    echo "Верно! Задание ${TITLE:-$ID} решено."
    exit 0
  fi
done

echo "Не подходит. Формат правильный, но это не флаг."
echo "Возможно, вы нашли приманку: в VaultNotes их несколько."
exit 2
