# Флаги VaultNotes

В VaultNotes спрятано 14 флагов. Каждая уязвимость в статьях разбирается по одной схеме: **как нашли агентом → почему это дыра → как надо было сделать**.

Названия уязвимостей спрятаны под спойлерами: если хотите пройти VaultNotes сами, не раскрывайте их.

| # | Часть | Область | Уязвимость |
|---|---|---|---|
| 1 | 2 | Память | <details><summary>спойлер</summary>Токен сессии остаётся в памяти после выхода</details> |
| 2 | 2 | Память | <details><summary>спойлер</summary>Пароль хранится в `String`</details> |
| 3 | 2 | Память | <details><summary>спойлер</summary>Номер карты живёт в in-memory кэше ответа сети</details> |
| 4 | 2 | Логи | <details><summary>спойлер</summary>Ответ сервера с токеном пишется в лог</details> |
| 5 | 3 | Приватность | <details><summary>спойлер</summary>Встроенный SDK читает `ANDROID_ID` и список приложений до согласия</details> |
| 6 | 3 | Криптография | <details><summary>спойлер</summary>Заметки шифруются AES/ECB ключом из кода</details> |
| 7 | 3 | Сеть | <details><summary>спойлер</summary>Пустой `X509TrustManager` «для тестового сервера»</details> |
| 8 | 3 | Платформа | <details><summary>спойлер</summary>`addJavascriptInterface` во WebView со справкой, URL из Intent</details> |
| 9 | 3 | Платформа | <details><summary>спойлер</summary>Mutable `PendingIntent` и неявный Intent с данными</details> |
| 10 | 4 | Код | <details><summary>спойлер</summary>Премиум-код скачивается зашифрованным и грузится из памяти</details> |
| 11 | 4 | Код | <details><summary>спойлер</summary>Секрет «спрятан» в `libvault.so`</details> |
| 12 | 4 | Логика | <details><summary>спойлер</summary>Проверка PIN и премиума только на клиенте</details> |
| 13 | 3 | Хранение | <details><summary>спойлер</summary>Поиск по заметкам собирает SQL конкатенацией</details> |
| 14 | 3 | Хранение | <details><summary>спойлер</summary>База открыта без шифрования, «шифрованная» таблица использует ключ из кода</details> |

Флаги подобраны так, чтобы покрыть основные группы требований OWASP MASVS: хранение, криптография, сеть, платформа и устойчивость к реверсу.
