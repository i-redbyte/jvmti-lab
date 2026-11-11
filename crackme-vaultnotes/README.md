# crackme-vaultnotes

> Хотите пройти VaultNotes сами? Не заглядывайте в `agents/`, `auditor/` и под спойлеры в `FLAGS.md`, пока не найдёте флаги своими инструментами.

Подопытное приложение **VaultNotes** (в статьях — HackMe): «защищённые» заметки с входом по PIN, поиском, премиум-функциями и справкой во WebView.

> ⚠️ Приложение уязвимо нарочно. Его код показывает, как делать **не нужно**: не переносите его в настоящие проекты.

## Модули

| Модуль | Что внутри |
|---|---|
| [`app`](app) | экраны, премиум, `src/main/cpp` — нативная библиотека `libvault.so` |
| [`premium`](premium) | премиум-функции на Java; в APK попадают зашифрованным DEX в `assets/premium.bin` |
| [`analytics-sdk`](analytics-sdk) | встроенный «сторонний» SDK аналитики (появится в следующих частях) |
| [`data`](data) | хранилище на Room (появится в следующих частях) |

## Сборка

Нужны JDK 17+, Android SDK с NDK `30.0.16248370` и build-tools `37.0.0`. Путь к SDK — в `local.properties` (`sdk.dir=...`) или в `ANDROID_HOME`.

```bash
./gradlew :app:assembleAudit
adb install -r app/build/outputs/apk/audit/app-audit.apk
```

| Тип сборки | R8 | debuggable | Зачем |
|---|---|---|---|
| `debug` | нет | да | обычная разработка |
| `audit` | да (minify + full mode) | да | против этой сборки работают агенты |
| `release` | да | нет | как в магазине |

У `audit` флаг debuggable выставлен в [`src/audit/AndroidManifest.xml`](app/src/audit/AndroidManifest.xml), а не через `isDebuggable` в Gradle: для debuggable-типов сборки AGP отключает обфускацию и оптимизации R8.

PIN для входа: `4821`.

## Параметры

- `applicationId`: `dev.redbyte.vaultnotes`
- `minSdk` 28: с Android 9 есть `Debug.attachJvmtiAgent` и `cmd activity attach-agent`.
- ABI: `arm64-v8a` (телефоны и эмулятор на Mac с Apple Silicon) и `x86_64` (эмулятор на Intel).
