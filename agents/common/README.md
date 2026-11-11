# common

Общий каркас агентов, только заголовки (`include/agent/`):

| Файл | Что внутри |
|---|---|
| `jvmti_ptr.h` | `JvmtiPtr` / `JvmtiString`: RAII для памяти JVMTI, сам вызывает `Deallocate` |
| `raw_monitor.h` | `RawMonitor` и `MonitorLock`: raw monitor JVMTI и блокировка в стиле `synchronized {}` |
| `jvmti_check.h` | `check()`: проверка `jvmtiError` с именем ошибки в logcat |
| `names.h` | имена классов и загрузчиков в привычном виде (`dev.redbyte.Foo`) |
| `log.h` | `AGENT_LOGI/W/E`: logcat с проверкой формата, как у `printf` |
| `json.h` | минимальный JSON для отчётов (JSON Lines) |
| `report.h` | файл отчёта в каталоге приложения |
| `options.h` | разбор строки опций агента |

## jvmti.h

В NDK нет заголовка `jvmti.h`, поэтому он лежит здесь: `include/jvmti.h` взят из OpenJDK 17 и распространяется под его лицензией (GPLv2 с Classpath Exception, см. шапку файла). ART реализует JVMTI 1.2 из того же интерфейса, поэтому агенты запрашивают `JVMTI_VERSION_1_2`.
