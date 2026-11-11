package dev.redbyte.vaultnotes.notes

import dev.redbyte.vaultnotes.vault.VaultNative
import java.security.MessageDigest

/** Заметки пока живут в памяти. Хранилище на Room появится в следующих частях. */
class NotesRepository {

    private val notes = mutableListOf(
        "Купить молоко",
        "Пароль от Wi-Fi на даче: на роутере",
        "Идея статьи: JVMTI в Android",
    )

    /** Подпись текущего набора заметок для будущей синхронизации с сервером. */
    var syncSignature: String = ""
        private set

    fun all(): List<String> = notes.toList()

    fun add(text: String) {
        if (text.isNotBlank()) notes += text.trim()
        sign()
    }

    fun sign() {
        val digest = MessageDigest.getInstance("SHA-256")
        digest.update(VaultNative.syncToken().toByteArray())
        notes.forEach { digest.update(it.toByteArray()) }
        syncSignature = digest.digest().joinToString("") { "%02x".format(it) }
    }
}
