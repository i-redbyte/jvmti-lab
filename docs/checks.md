# Что проверить на устройстве

- [ ] Точные API-уровни для `cmd activity attach-agent`, `am start --attach-agent` и `Debug.attachJvmtiAgent` на актуальных версиях Android. Куда класть `.so` агента при подключении снаружи (`run-as`, каталог данных приложения).
- [ ] Какие capabilities реально поддерживает ART (`GetPotentialCapabilities` на устройстве): `can_generate_breakpoints`, `can_access_local_variables`, `can_tag_objects`, `can_generate_native_method_bind_events`, `can_retransform_classes`, `can_force_early_return`.
- [ ] Поддерживает ли ART колбэки для строк и массивов в `IterateThroughHeap`.
- [ ] Можно ли ставить точки останова на классы фреймворка из boot image и какие классы ART не даёт переопределять.
- [ ] В каком виде ART отдаёт байты в `ClassFileLoadHook` для классов из `InMemoryDexClassLoader`.
- [ ] Актуальная версия NDK и статус требования 16 КБ страниц.
