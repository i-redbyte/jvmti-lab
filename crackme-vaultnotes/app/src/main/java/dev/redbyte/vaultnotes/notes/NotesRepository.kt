package dev.redbyte.vaultnotes.notes

/** Заметки пока живут в памяти. Хранилище на Room появится в следующих частях. */
class NotesRepository {

    private val notes = mutableListOf(
        "Купить молоко",
        "Пароль от Wi-Fi на даче: на роутере",
        "Идея статьи: JVMTI в Android",
    )

    fun all(): List<String> = notes.toList()

    fun add(text: String) {
        if (text.isNotBlank()) notes += text.trim()
    }
}
