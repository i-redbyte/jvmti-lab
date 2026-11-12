# ТЗ: обвязка демо-приложения «Супер-Мани» (SuperMoney)

Документ для Android-разработчика. Самодостаточен: читать статьи цикла не нужно. Код приложения лежит в репозитории в папке `crackme-vaultx/` (её нужно переименовать в `crackme-supermoney/`, см. §3).

---

## 1. Контекст

«Супер-Мани» — демонстрационное инвест-приложение под Android: инвест-счёт, портфель, операции (сделки), баланс, пополнение картой, подписка Pro (триал → Pro). Это **учебный HackMe/CTF** для цикла статей по безопасности: приложение **намеренно** содержит типовые уязвимости, захардкоженные секреты и строки-приманки под 14 заданий. Задача разработчика — аккуратно реализовать эту «обвязку» (экраны, репозитории, хранилище, сеть, нативный мост) так, чтобы каждое задание решалось ровно как описано ниже.

Важно: уязвимости здесь — **фича, а не баг**. Не «чините» их и не добавляйте защит, которых в ТЗ нет. Знание JVMTI/реверса не требуется: вы делаете обычное Android-приложение, просто с заранее заданными «дырами».

Каждое задание (таск) даёт строку-флаг вида `vault{...}`. Значения флагов подставляет сборка (см. §6), в исходники их вписывать нельзя.

---

## 2. Стек и факты

- **Язык/UI:** Kotlin, Jetpack Compose.
- **minSdk 28**, compileSdk/targetSdk — из `gradle/libs.versions.toml`.
- **Нативная часть:** NDK/CMake, библиотека `libvault.so` (пакет `dev.redbyte.supermoney`). Мост — `VaultNative` (готов, см. §3).
- **Пакет:** `dev.redbyte.supermoney`, applicationId тот же.
- **Сервер:** Ktor, **уже готов**, поднимается из `server/`. Приложение ходит на него по HTTPS с самоподписанным сертификатом. Адрес с эмулятора — `https://10.0.2.2:8443` (loopback хоста), отладочный HTTP без TLS — `http://10.0.2.2:8080`.
- **Аккаунт:** логин `demo`, пароль `swordfish-7Gx` (показаны на экране входа, это не секрет).
- **PIN локальной разблокировки:** `4821` (в README, не флаг).
- **Дизайн:** яркая зелёно-золотая палитра (деньги/рост), **светлая и тёмная** темы через Compose Material 3. Экран входа, экран согласия, PIN, портфель.

**Контракт сервера (готов, менять не надо, только вызывать).** Текущие маршруты: `POST /auth/login` (→ токен, похожий на JWT, `expiresIn`), `POST /auth/logout`, `GET /balance`/портфель, `POST /deposit/card` (пополнение картой — **в ответе эхом возвращает номер карты целиком**), `GET /pro/status` (→ `{"pro": false}`), `POST /analytics` (приём событий аналитики). На сервере **нет ни одного флага** — все флаги в APK. Точные имена полей ответа сверяйте с кодом `server/`; если какой-то маршрут в `server/` ещё назван по-старому (`/payments`, `/premium/status`), используйте то, что реально отдаёт сервер, и сообщите координатору — финальное выравнивание имён на сервере согласуется отдельно.

---

## 3. Что уже готово и что НЕ входит в вашу задачу

**Готово и трогать не нужно:**
- **Ktor-сервер** (`server/`).
- **JVMTI-агенты и аудитор** (`agents/`, `auditor/`) — их делает другой человек. Вам они не нужны и не касаются.
- **CTF-инфраструктура** (`ctf/`): генерация флагов, хэши, карточки заданий, `check-flag.sh`.
- **Нативная часть `libvault.so`** (`crackme-vaultx/app/src/main/cpp/vault.cpp`) и мост `VaultNative` — реализованы: `checkPin`, `proModuleKey`, нативный секрет (см. §3 про переименование). JNI-имена заведены под пакет.

**Уже лежит локально, без коммита (НЕ делайте заново):**
- Папка `crackme-vaultnotes` переименована в `crackme-vaultx`, пакет `dev.redbyte.vaultx`.
- **Pro-модуль** `crackme-vaultx/pro/`: классы `TaxReportExporter`, `CsvEscaper`, `ProBadge`, лицензия `ProLicense` (генерируется при сборке) — это готовая реализация rev-01.
- `app/.../pro/ProModuleLoader.kt` (InMemoryDexClassLoader, расшифровка `pro-module.bin`), `app/.../vault/VaultNative.kt`.
- `app/build.gradle.kts`: сборка Pro-модуля в зашифрованный `assets/pro-module.bin` + генератор объекта `Keys` из `ctf/secrets/flags.properties`.
- SDK аналитики **Trackly** — модуль `analytics-sdk/` (`com.trackly.sdk.Trackly`).
- Заготовка `app/.../ui/MainActivity.kt`.

**Обязательное переименование (сделать первым делом, единым проходом):**
1. Папку `crackme-vaultx/` → `crackme-supermoney/`.
2. Пакет `dev.redbyte.vaultx` → `dev.redbyte.supermoney` (Kotlin, Java, C++ JNI-имена `Java_dev_redbyte_supermoney_...`, `namespace`/`applicationId`, манифесты, `FindClass` в `vault.cpp`).
3. `signingSeed` → `reportSigningKey` в `VaultNative.kt` и в `vault.cpp` (и в RegisterNatives-таблице).
4. В генераторе `Keys` (в `app/build.gradle.kts`) — выходной пакет `dev/redbyte/supermoney/core/Keys.kt` и gradle-property `vaultx.proModuleKeyHex` → `supermoney.proModuleKeyHex`.

После переименования всё ниже использует `dev.redbyte.supermoney`.

---

## 4. Поимённая спецификация заданий

Общие правила:
- Все классы с уязвимостями — в модуле приложения (`crackme-supermoney/app/...`), кроме Pro-модуля (`pro/`, готов) и SDK (`analytics-sdk/`, готов).
- **Флаги берутся из объекта `Keys`** (генерируется сборкой), каждому заданию — свой аксессор (таблица в §6). Строку флага в код не вписывать.
- Сборку вести в build type **`audit`** (как release c R8, но debuggable в манифесте варианта) — он уже настроен.
- «Приманка» (decoy) — строка `vault{...}` на видном месте, которую `check-flag.sh` НЕ засчитывает. Делать её «мёртвой»: константа, которую никто не кладёт в живые объекты.

### Часть 2. Память (mem-01…mem-04)

**mem-01 — сессия живёт после логаута.**
Файлы: `auth/SessionManager.kt`, `auth/Session.kt`.
- `object SessionManager` с полем `current: Session?` (UI смотрит на него) и **вторым** полем `lastSession: Session?` «для быстрого повторного входа». `logout()` обнуляет `current`, но **не трогает** `lastSession`.
- `Session` хранит токен (JWT с сервера), имя пользователя (`demo`) и поле со **секретом** `Keys.deviceSecret()` (это флаг mem-01).
- Приёмка: после логина+логаута объект `Session` c токеном и флагом **остаётся достижим** из `SessionManager` (синглтон). Рядом в объекте лежат строки `demo` и JWT.

**mem-02 — пароль остаётся в памяти.**
Файл: `auth/LoginViewModel.kt` (+ `auth/AuthRepository.kt`).
- `password` хранится как `String` в `mutableStateOf`, после входа **не очищается** (комментарий «чтобы не вводить заново при обрыве сети»).
- Рядом с удерживаемым паролем (в том же ViewModel/объекте) положить `Keys.walletPassphrase()` (флаг mem-02).
- Пароль `swordfish-7Gx` намеренно непохож на логин — он служит «меткой» в памяти. Приёмка: после входа и логаута строка пароля и флаг достижимы в куче.

**mem-03 — кэш пополнения помнит номер карты.**
Файл: `payments/DepositCache.kt`.
- `object DepositCache { var lastResponse: DepositResponse? = null }` — кэширует **весь** ответ `POST /deposit/card`, включая номер карты целиком (сервер возвращает его эхом).
- В разобранный ответ (`DepositResponse`) добавить поле, которого сервер не присылал, со значением `Keys.depositReference()` (флаг mem-03).
- Экран пополнения: ввод карты (тестовый номер вида `4111 1111 1111 1111`), «оплата», ответ кладётся в `DepositCache`. Кэш не чистится. Приёмка: после пополнения номер карты и флаг достижимы из синглтона.

**mem-04 — логирование тела ответа.**
Файл: `auth/AuthRepository.kt`.
- В `login()` после получения ответа: `Log.d("SmNet", "login response: $body")` — логируется тело ответа целиком (токен + поле с `Keys.debugKey()` = флаг mem-04).
- Приёмка: при входе в `logcat` уходит строка с JWT и флагом mem-04.

### Часть 3. Поведение

**priv-01 — SDK читает идентификаторы до согласия.**
Файлы: `App.onCreate` (Application), модуль `analytics-sdk/` (**Trackly, готов** — проверить/дореализовать поведение).
- `Trackly.init(context)` вызывается в `Application.onCreate` — **до** экрана согласия. Внутри читает `Settings.Secure.getString(cr, "android_id")` и `packageManager.getInstalledApplications(0)`.
- Собранное событие (перед `POST /analytics`) содержит поле с `Keys.analyticsWriteKey()` (флаг priv-01).
- Экран согласия показывается позже; инициализация SDK должна происходить **раньше** и независимо от согласия. Приёмка: при холодном старте без согласия SDK уже прочитал `ANDROID_ID` и подготовил событие с флагом.

**crypto-01 — шифрование локального кэша ключом из кода.**
Файл: `crypto/VaultCrypto.kt`.
- `object VaultCrypto` шифрует/расшифровывает локальный кэш (офлайн-копию портфеля) через `Cipher.getInstance("AES/ECB/PKCS5Padding")` и `SecretKeySpec(KEY, "AES")`, где `KEY` — **захардкоженный** 16-байтный ключ.
- **Важно (критерий приёмки):** этот ключ должен быть **байт-в-байт** тем же, которым сборка запечатала `Keys.SEALED_COLD_WALLET` (сейчас в генераторе `app/build.gradle.kts` это строка `vaultx-walletkey`; при переименовании держите ключ в коде и в генераторе синхронными). Расшифровав `Keys.SEALED_COLD_WALLET` этим ключом, получаем строку `cold-wallet: vault{...}` — внутри флаг crypto-01.
- Приёмка: `VaultCrypto.decrypt(Keys.SEALED_COLD_WALLET)` отдаёт текст с флагом crypto-01; режим именно `AES/ECB`, ключ виден в аргументах `SecretKeySpec`/`Cipher.init`.

**net-01 — клиент доверяет любому сертификату (+ приманка).**
Файл: `net/TlsSetup.kt`.
- Два `X509TrustManager`: `TrustAllManager` с **пустым** `checkServerTrusted` (доверяет всем) и второй, «строгий» на вид (`StrictPinningManager` — выглядит как проверяющий, но в `SSLContext.init` **не попадает**) — это приманка.
- В `SSLContext.init(null, arrayOf(TrustAllManager()), SecureRandom())` уходит именно пустой менеджер (иначе HTTPS к самоподписанному серверу не пройдёт).
- Рядом с местом сборки пустого менеджера — `Keys.trustTicket()` (флаг net-01).
- Приёмка: приложение успешно ходит на самоподписанный `https://10.0.2.2:8443`; реально подключён пустой TrustManager; «строгий» существует, но не используется.

**plat-01 — WebView-мост + URL из Intent.**
Файл: `help/HelpActivity.kt`.
- Экран справки на `WebView`: `settings.javaScriptEnabled = true`, `addJavascriptInterface(VaultBridge(), "VaultBridge")`, `loadUrl(...)` где URL **берётся из `Intent` extra** (`help_url`), то есть управляется снаружи.
- У моста `VaultBridge` есть `@JavascriptInterface`-метод (например `supportInfo()`), который возвращает `Keys.supportToken()` (флаг plat-01) — метод зря выставлен наружу.
- Приёмка: внешний Intent с произвольным `help_url` открывает справку; JS со страницы через `VaultBridge` получает флаг plat-01.

**plat-02 — изменяемый PendingIntent с неявным Intent.**
Файл: `notify/OrderNotifier.kt`.
- Уведомление об операции строит `PendingIntent.getBroadcast(..., flags=0)` (**без** `FLAG_IMMUTABLE`) поверх **неявного** Intent (`action = "dev.redbyte.supermoney.SHARE"`, без явного компонента). В `extras` — данные операции и `token`, а также `Keys.orderNotifyToken()` (флаг plat-02).
- Приёмка: `PendingIntent` изменяемый, Intent неявный, extras содержат флаг plat-02.

**store-01 — SQL-инъекция в поиске.**
Файлы: `data/LedgerDatabase.kt`, `data/LedgerRepository.kt`.
- Поиск по операциям строит SQL **конкатенацией** пользовательского ввода: `rawQuery("SELECT * FROM orders WHERE memo LIKE '%$query%'", null)`.
- В БД есть «скрытая» запись/строка (например операция с `memo`, не показываемая в обычном UI), в которой лежит `Keys.hiddenOrderNote()` (флаг store-01). Инъекция `' OR '1'='1' --` возвращает её.
- Приёмка: обычный поиск безопасен по виду, но инъекция вытаскивает скрытую запись с флагом.

**store-02 — SQLCipher с ключом из кода + основная база открыта.**
Файлы: `data/LedgerDatabase.kt` (+ зависимость SQLCipher/`SupportFactory`).
- «Защищённую» часть открывают через `SupportFactory(passphrase)`, где `passphrase` — **захардкоженный** ключ (`char[]`/`byte[]`), передаётся в **конструктор фабрики**. Основную базу `orders.db` открывают **без** шифрования.
- Ключ (или строка рядом) = `Keys.walletBackupCode()` (флаг store-02): либо флаг в расшифрованной таблице, либо ключ-приманка + флаг внутри. Согласуйте одну из схем и зафиксируйте.
- Приёмка: ключ виден в аргументе конструктора `SupportFactory`; основная база без шифрования; флаг store-02 достаётся.

### Часть 4. Скрытое

**rev-01 — динамический DEX (готово).**
Pro-модуль `pro/` уже реализует это: `TaxReportExporter` и др. собираются в DEX, шифруются в `assets/pro-module.bin`, грузятся `ProModuleLoader` через `InMemoryDexClassLoader`. Флаг rev-01 = строка лицензии `ProLicense` (подставляется при сборке). Экран должен **вызывать** Pro-функцию (кнопка «Налоговый отчёт ★»), чтобы модуль реально подгружался.
- Приёмка: нажатие «Налоговый отчёт ★» грузит `TaxReportExporter` из памяти; в APK (`classes*.dex`) этого класса нет, в `assets` — только `pro-module.bin`.

**rev-02 — секрет в нативной библиотеке.**
Файлы: `data/LedgerRepository.kt` (+ `vault/VaultNative.kt`, нативка готова).
- `VaultNative.reportSigningKey()` — нативный метод (RegisterNatives, без экспортируемого символа), возвращает ключ HMAC для подписи **серверного отчёта**; строка замаскирована в `.so` побайтово. Флаг rev-02 = это значение.
- Нужен **Java-вызыватель**: `LedgerRepository.signReport()` зовёт `VaultNative.reportSigningKey()` и кладёт результат в **локальную переменную** (используется для подписи отчёта по операциям). Метод вызывается в реальном сценарии (например при открытии портфеля/формировании отчёта).
- Приманка: в `.so` уже есть открытая `vault{...}` в `kDebugSeed` (нативка), её `check-flag.sh` не засчитывает.
- Приёмка: `signReport()` реально вызывает нативный метод и держит результат в локальной переменной (чтобы его можно было перехватить на границе JNI).

**rev-03 — клиентская проверка Pro.**
Файл: `pro/ProGate.kt`.
- `object ProGate { fun isProUnlocked(): Boolean }` — **клиентское** решение о Pro (лицензия из Pro-модуля / промокод / что угодно локальное), игнорирующее серверный `GET /pro/status` (`{"pro": false}`).
- Метод-**обманка** `isProUnlockedFake()`, подмена которого ничего не меняет (реальное решение принимает `isProUnlocked()`); результат `isProUnlocked()` реально доходит до разблокировки Pro-экрана.
- Pro-экран (доступный только при `isProUnlocked() == true`) показывает `Keys.proWelcomeText()` (флаг rev-03).
- Приёмка: при честном ответе сервера Pro закрыт; Pro-экран с флагом открывается только когда `isProUnlocked()` вернул `true`.

---

## 5. Экраны (`ui/MainActivity.kt` и связанные)

Поток: **экран согласия** (аналитика уже инициализирована до него — priv-01) → **вход** (`demo`/`swordfish-7Gx`) → **PIN** (`4821`, первый ввод загружает `libvault.so`) → **портфель**: баланс, список операций, **поиск** (store-01), **пополнение картой** (mem-03), кнопка **«Налоговый отчёт ★»** (rev-01), раздел **Pro** (rev-03), **справка** (plat-01, WebView), уведомление об операции (plat-02), **выход** (mem-01).
- Тема: зелёно-золотая, Material 3, светлая + тёмная (`isSystemInDarkTheme()`), явный фон.
- Экран не должен «чинить» уязвимости (не очищать сессию по-настоящему в mem-01 и т.д.).

---

## 6. Флаги: как подключать

Флаги **не хранятся в исходниках**. Их значения лежат вне репозитория (`ctf/secrets/flags.properties`, создаётся `ctf/tools/gen-flags.sh`), а сборка генерирует объект `dev.redbyte.supermoney.core.Keys` с замаскированными строками. Берите флаг через соответствующий аксессор:

| Задание | Источник флага в коде |
|---|---|
| mem-01 | `Keys.deviceSecret()` |
| mem-02 | `Keys.walletPassphrase()` |
| mem-03 | `Keys.depositReference()` |
| mem-04 | `Keys.debugKey()` |
| priv-01 | `Keys.analyticsWriteKey()` |
| crypto-01 | `Keys.SEALED_COLD_WALLET` (расшифровать `VaultCrypto` тем же ключом) |
| net-01 | `Keys.trustTicket()` |
| plat-01 | `Keys.supportToken()` |
| plat-02 | `Keys.orderNotifyToken()` |
| store-01 | `Keys.hiddenOrderNote()` |
| store-02 | `Keys.walletBackupCode()` |
| rev-01 | `ProLicense` (Pro-модуль, подставляется сборкой) |
| rev-02 | `VaultNative.reportSigningKey()` (нативка) |
| rev-03 | `Keys.proWelcomeText()` |

Приманки (`vault{...}`, не засчитываются): открытая строка в `.so` (`kDebugSeed`), «строгий» TrustManager без реального подключения (net-01), метод `isProUnlockedFake()` (rev-03), ключ-приманка store-02. Приманки держать «мёртвыми».

---

## 7. Как проверить (критерий готовности)

1. Сгенерировать флаги: из корня репозитория `./ctf/tools/gen-flags.sh` (создаст `ctf/secrets/flags.properties`).
2. Поднять сервер: `./scripts/run-server.sh`.
3. Собрать APK варианта `audit`: `./crackme-supermoney/gradlew :app:assembleAudit` (или через Android Studio). Убедиться, что `pro-module.bin` и `Keys.kt` сгенерировались.
4. Установить на эмулятор (arm64 на Apple Silicon / x86_64 на Intel), пройти сценарий: согласие → вход → PIN → портфель → пополнение → поиск → Налоговый отчёт → Pro → справка → выход.
5. Проверить все **14 флагов** через `./ctf/check-flag.sh 'vault{...}'` — каждый флаг должен извлекаться ровно тем способом, что описан в §4, и засчитываться; приманки — **не** засчитываться.

Готово, когда все 14 заданий воспроизводятся на эмуляторе и приманки не ломают игру. Вопросы по именам/местам флагов — к координатору проекта.
