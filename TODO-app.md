# TODO: приложение «Супер-Мани»

Код приложения в `crackme-vaultx/` (папку стоит переименовать в `crackme-supermoney`). Что и где делать, описано в статьях цикла (разделы с ID заданий) и в `jvmti/rename-map.md` в библиотеке проекта. Ниже только карта файлов.

## Уже лежит локально, без коммита

- Папка `crackme-vaultnotes` переименована в `crackme-vaultx`, пакет `dev.redbyte.vaultx`.
- Pro-модуль: `pro/` (`TaxReportExporter`, `CsvEscaper`, `ProBadge`, лицензия `ProLicense` генерируется при сборке).
- `app/.../pro/ProModuleLoader.kt`, `app/.../vault/VaultNative.kt` (`checkPin`, `proModuleKey`, `signingSeed`).
- `app/src/main/cpp/vault.cpp`: JNI-имена под новый пакет, `signingSeed`.
- `app/build.gradle.kts`: Pro-модуль, генератор `Keys.kt` из `ctf/secrets/flags.properties`.
- `analytics-sdk/` (SDK Trackly).

Осталось переименовать: `vaultx` → `supermoney` в пакете, `signingSeed` → `reportSigningKey` в Kotlin и C++, а также имена функций-аксессоров в генераторе `Keys`.

## Часть 2 (память)

| Задание | Файл | Раздел статьи |
|---|---|---|
| mem-01 | `auth/SessionManager.kt`, `auth/Session` | part2, «mem-01» |
| mem-02 | `auth/LoginViewModel.kt` | part2, «mem-02» |
| mem-03 | `payments/DepositCache.kt` (ответ `/deposit/card`) | part2, «mem-03» |
| mem-04 | `auth/AuthRepository.kt` | part2, «mem-04» |

## Часть 3 (поведение)

| Задание | Файл | Раздел статьи |
|---|---|---|
| priv-01 | `App.onCreate` + `analytics-sdk` | part3, раунд 4 |
| crypto-01 | `crypto/VaultCrypto.kt` | part3, раунд 5 |
| net-01 | `net/TlsSetup.kt` | part3, раунд 6 |
| plat-01, plat-02 | `help/HelpActivity.kt`, `notify/OrderNotifier.kt` | part3, раунд 7 |
| store-01, store-02 | `data/LedgerDatabase.kt`, `data/LedgerRepository.kt` | part3, раунд 8 |

## Часть 4

| Задание | Файл | Раздел статьи |
|---|---|---|
| rev-01 | готово (`pro/`, `ProModuleLoader`) | part4, раунд 1 |
| rev-02 | `data/LedgerRepository.signReport()` вызывает `VaultNative.reportSigningKey()` | part4, раунд 2 |
| rev-03 | `pro/ProGate.isProUnlocked()` | part4, раунд 3 |

## Экраны

`ui/MainActivity.kt`: согласие → вход → PIN → портфель (баланс, операции, поиск, «Налоговый отчёт ★», Pro, справка, выход). Тема в зелёно-золотых тонах, светлая и тёмная.

## Флаги

Значения подставляет Gradle из `ctf/secrets/flags.properties` через объект `Keys`. Строки флагов в исходники не вписывать.
