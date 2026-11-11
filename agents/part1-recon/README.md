# part1-recon

Первый агент цикла: разведка VaultNotes.

- `ClassLoad` / `ClassPrepare` + `GetClassLoader`: какие классы откуда пришли. Находка: часть классов загружена через `InMemoryDexClassLoader`.
- `NativeMethodBind` + `dladdr`: таблица «native-метод → библиотека → символ». Находка: проверка PIN уходит в `libvault.so`.
- Вывод в logcat и JSON: «карта местности» для следующих частей.
