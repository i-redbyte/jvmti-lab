# part1-recon

Первый агент цикла: разведка VaultNotes.

- Печатает возможности JVMTI, которые ART даёт на этом устройстве (`GetPotentialCapabilities`).
- `GetLoadedClasses` + `ClassPrepare` + `GetClassLoader`: какой загрузчик принёс каждый класс приложения. Классы не из APK попадают в лог с пометкой.
- `NativeMethodBind` + `dladdr`: таблица «native-метод → библиотека → смещение и символ».
- Вывод в logcat (тег `jvmti-recon`) и в `files/jvmti-reports/recon.jsonl`.

## Запуск

```bash
./scripts/build-agents.sh
# к уже запущенному приложению:
./scripts/attach-agent.sh agents/build/arm64-v8a/part1-recon/libagent_recon.so
# или с перезапуском, чтобы увидеть всё с самого начала:
./scripts/attach-agent.sh --start agents/build/arm64-v8a/part1-recon/libagent_recon.so
adb logcat -s jvmti-recon
./scripts/pull-report.sh
```

Опции агента (второй аргумент `attach-agent.sh`, через запятую):

| Опция | Что делает |
|---|---|
| `verbose` | печатать в logcat каждый класс приложения, а не только классы не из APK |
| `out=<каталог>` | куда писать отчёт вместо `files/jvmti-reports` |

## Что важно знать

- Native-методы, связанные до подключения агента, `NativeMethodBind` уже не покажет. VaultNotes загружает `libvault.so` при первом вводе PIN, поэтому агент удобно подключать на экране PIN или через `--start`.
- При `--start` агент подключается раньше, чем загружаются классы приложения, поэтому они приходят событиями `ClassPrepare`, а не из `GetLoadedClasses`.
