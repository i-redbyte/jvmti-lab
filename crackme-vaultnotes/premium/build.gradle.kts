// Премиум-модуль на чистой Java: после сборки он превращается в DEX и шифруется.
// Kotlin здесь не используется, чтобы DEX не зависел от kotlin-stdlib, которую R8 ужимает в приложении.
plugins {
    `java-library`
}

java {
    toolchain {
        languageVersion = JavaLanguageVersion.of(17)
    }
}

dependencies {
    // Ничего: премиум-код получает заметки обычными коллекциями Java.
}
