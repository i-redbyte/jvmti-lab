# premium

Премиум-функции VaultNotes на чистой Java. Gradle собирает модуль в jar, превращает его в DEX через `d8`, шифрует AES и кладёт в `assets/premium.bin` приложения. Приложение расшифровывает модуль в памяти и загружает его через `InMemoryDexClassLoader`.
