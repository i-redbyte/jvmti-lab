import java.io.ByteArrayOutputStream
import java.security.SecureRandom
import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.compose)
}

val premiumKeyHex = providers.gradleProperty("vaultnotes.premiumKeyHex").get()

// Флаги CTF лежат вне репозитория: ctf/secrets/flags.properties (создаёт ctf/tools/gen-flags.sh).
val ctfFlags: Map<String, String> = run {
    val file = rootDir.resolve("../ctf/secrets/flags.properties")
    if (!file.isFile) {
        throw GradleException("Нет флагов CTF: ${file.normalize()}. Запустите ./ctf/tools/gen-flags.sh из корня репозитория.")
    }
    file.readLines().filter { "=" in it }.associate { it.substringBefore('=') to it.substringAfter('=') }
}

android {
    namespace = "dev.redbyte.vaultnotes"
    compileSdk = libs.versions.compileSdk.get().toInt()
    ndkVersion = libs.versions.ndk.get()
    buildToolsVersion = libs.versions.buildTools.get()

    defaultConfig {
        applicationId = "dev.redbyte.vaultnotes"
        minSdk = libs.versions.minSdk.get().toInt()
        targetSdk = libs.versions.targetSdk.get().toInt()
        versionCode = 1
        versionName = "0.1"

        // Эмулятор на Mac с Apple Silicon и телефоны: arm64. Эмулятор на Intel: x86_64.
        ndk { abiFilters += listOf("arm64-v8a", "x86_64") }
        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_static",
                    "-DVAULT_PREMIUM_KEY_HEX=$premiumKeyHex",
                    "-DVAULT_SYNC_TOKEN=${ctfFlags.getValue("rev-02")}",
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
        // Сборка, против которой работают агенты: обфусцирована R8, как release,
        // но остаётся debuggable, иначе JVMTI-агент к ней не подключится.
        // debuggable выставлен в src/audit/AndroidManifest.xml (почему — см. там).
        create("audit") {
            initWith(getByName("release"))
            signingConfig = signingConfigs.getByName("debug")
            matchingFallbacks += listOf("release")
        }
    }

    buildFeatures {
        compose = true
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

// --- Премиум-модуль: jar → DEX (d8) → AES → assets/premium.bin ---

val premiumJar: Configuration = configurations.create("premiumJar") {
    isCanBeConsumed = false
    isTransitive = false
}

dependencies {
    premiumJar(project(":premium"))

    implementation(platform(libs.compose.bom))
    implementation(libs.compose.ui)
    implementation(libs.compose.foundation)
    implementation(libs.compose.material3)
    implementation(libs.activity.compose)
}

abstract class PackPremiumTask : DefaultTask() {
    @get:InputFiles
    abstract val jars: ConfigurableFileCollection

    @get:Input
    abstract val d8Path: Property<String>

    @get:Input
    abstract val keyHex: Property<String>

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @get:Inject
    abstract val exec: ExecOperations

    @TaskAction
    fun pack() {
        val dexDir = temporaryDir.resolve("dex").apply { deleteRecursively(); mkdirs() }
        val log = ByteArrayOutputStream()
        exec.exec {
            commandLine(d8Path.get(), "--release", "--min-api", "28", "--output", dexDir.absolutePath)
            args(jars.files.map { it.absolutePath })
            environment("JAVA_HOME", System.getProperty("java.home"))
            standardOutput = log
            errorOutput = log
        }
        val dex = dexDir.resolve("classes.dex").readBytes()

        // Формат premium.bin: 16 байт IV, затем DEX, зашифрованный AES/CBC.
        val key = keyHex.get().chunked(2).map { it.toInt(16).toByte() }.toByteArray()
        val iv = ByteArray(16).also { SecureRandom().nextBytes(it) }
        val cipher = Cipher.getInstance("AES/CBC/PKCS5Padding")
        cipher.init(Cipher.ENCRYPT_MODE, SecretKeySpec(key, "AES"), IvParameterSpec(iv))
        val out = outputDir.get().asFile.apply { mkdirs() }.resolve("premium.bin")
        out.writeBytes(iv + cipher.doFinal(dex))
    }
}

val packPremium = tasks.register<PackPremiumTask>("packPremium") {
    jars.from(premiumJar)
    d8Path = androidComponents.sdkComponents.sdkDirectory.map {
        it.file("build-tools/${libs.versions.buildTools.get()}/d8").asFile.absolutePath
    }
    keyHex = premiumKeyHex
}

androidComponents {
    onVariants { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(packPremium, PackPremiumTask::outputDir)
    }
}
