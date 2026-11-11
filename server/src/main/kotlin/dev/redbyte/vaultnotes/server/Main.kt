package dev.redbyte.vaultnotes.server

import io.ktor.network.tls.certificates.buildKeyStore
import io.ktor.network.tls.certificates.saveToFile
import io.ktor.server.engine.connector
import io.ktor.server.engine.embeddedServer
import io.ktor.server.engine.sslConnector
import io.ktor.server.netty.Netty
import java.io.File
import java.security.KeyStore

private const val KEY_ALIAS = "vaultnotes"
private const val KEY_PASSWORD = "vaultnotes"

/**
 * Локальный сервер VaultNotes.
 *
 *  - https://0.0.0.0:8443 — основной адрес, самоподписанный сертификат;
 *  - http://0.0.0.0:8080  — тот же API без TLS, для отладки.
 *
 * Эмулятор видит компьютер по адресу 10.0.2.2.
 */
fun main() {
    val keyStoreFile = File("certs/vaultnotes.jks")
    val keyStore = loadOrCreateKeyStore(keyStoreFile)

    embeddedServer(
        Netty,
        configure = {
            connector { port = 8080 }
            sslConnector(
                keyStore = keyStore,
                keyAlias = KEY_ALIAS,
                keyStorePassword = { KEY_PASSWORD.toCharArray() },
                privateKeyPassword = { KEY_PASSWORD.toCharArray() },
            ) {
                port = 8443
                keyStorePath = keyStoreFile
            }
        },
        module = { vaultNotesApi(Store()) },
    ).start(wait = true)
}

// Самоподписанный сертификат создаётся при первом запуске.
private fun loadOrCreateKeyStore(file: File): KeyStore {
    if (file.isFile) {
        return KeyStore.getInstance("JKS").apply { file.inputStream().use { load(it, KEY_PASSWORD.toCharArray()) } }
    }
    file.parentFile.mkdirs()
    return buildKeyStore {
        certificate(KEY_ALIAS) {
            password = KEY_PASSWORD
            domains = listOf("localhost", "10.0.2.2")
        }
    }.also { it.saveToFile(file, KEY_PASSWORD) }
}
