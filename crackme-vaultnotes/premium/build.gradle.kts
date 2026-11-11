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

// Лицензия премиум-модуля генерируется при сборке из ctf/secrets/flags.properties.
abstract class GenerateLicenseTask : DefaultTask() {
    @get:InputFile
    abstract val flags: RegularFileProperty

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @TaskAction
    fun generate() {
        val license = flags.get().asFile.readLines()
            .first { it.startsWith("rev-01=") }
            .substringAfter('=')
        val dir = outputDir.get().asFile.resolve("dev/redbyte/vaultnotes/premium").apply { mkdirs() }
        dir.resolve("PremiumLicense.java").writeText(
            """
            |package dev.redbyte.vaultnotes.premium;
            |
            |final class PremiumLicense {
            |    static final String ID = "$license";
            |
            |    private PremiumLicense() {
            |    }
            |
            |    static boolean isActive() {
            |        return ID.startsWith("vault{");
            |    }
            |}
            |""".trimMargin()
        )
    }
}

val generateLicense = tasks.register<GenerateLicenseTask>("generateLicense") {
    flags = rootProject.layout.projectDirectory.file("../ctf/secrets/flags.properties")
    outputDir = layout.buildDirectory.dir("generated/sources/license")
}

sourceSets.main {
    java.srcDir(generateLicense)
}
