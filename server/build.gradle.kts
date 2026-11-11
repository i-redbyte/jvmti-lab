plugins {
    kotlin("jvm") version "2.4.21"
    application
}

val ktor = "3.5.2"

dependencies {
    implementation("io.ktor:ktor-server-core:$ktor")
    implementation("io.ktor:ktor-server-netty:$ktor")
    implementation("io.ktor:ktor-network-tls-certificates:$ktor")
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.9.0")
    implementation("ch.qos.logback:logback-classic:1.5.18")
}

kotlin {
    jvmToolchain(17)
}

application {
    mainClass = "dev.redbyte.vaultnotes.server.MainKt"
}

tasks.named<JavaExec>("run") {
    // Сертификат и ключ сервера живут рядом с исходниками, но в git не попадают.
    workingDir = projectDir
}
