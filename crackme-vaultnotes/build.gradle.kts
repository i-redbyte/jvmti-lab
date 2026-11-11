buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        // В AGP 9 Kotlin встроен в плагин Android, эта строка поднимает его до свежей версии.
        classpath(libs.kotlin.gradle.plugin)
    }
}

plugins {
    alias(libs.plugins.android.application) apply false
    alias(libs.plugins.kotlin.compose) apply false
}
