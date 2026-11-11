pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "vaultnotes"

// Само приложение VaultNotes.
include(":app")
// Премиум-модуль. В APK он попадает не как обычный код, а зашифрованным DEX в assets.
include(":premium")
