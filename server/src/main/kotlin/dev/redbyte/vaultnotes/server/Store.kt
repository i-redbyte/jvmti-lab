package dev.redbyte.vaultnotes.server

import java.security.SecureRandom
import java.util.concurrent.ConcurrentHashMap

/** Всё хранится в памяти: сервер учебный, после перезапуска состояние сбрасывается. */
class Store {
    // Единственный пользователь. Логин и пароль подсказаны на экране входа.
    private val users = mapOf("demo" to "demo")
    private val sessions = ConcurrentHashMap<String, String>()
    private val notes = ConcurrentHashMap<String, MutableList<String>>()
    private val random = SecureRandom()

    fun login(user: String, password: String): String? {
        if (users[user] != password) return null
        val token = newToken(user)
        sessions[token] = user
        return token
    }

    fun logout(token: String) {
        sessions.remove(token)
    }

    fun userOf(token: String?): String? = token?.let { sessions[it] }

    fun notes(user: String): List<String> = notes.getOrPut(user) { mutableListOf() }.toList()

    fun addNote(user: String, text: String) {
        notes.getOrPut(user) { mutableListOf() } += text
    }

    // Похож на JWT: заголовок.данные.подпись. Подпись случайная, сервер проверяет токен по таблице сессий.
    private fun newToken(user: String): String {
        val header = base64Url("""{"alg":"HS256","typ":"JWT"}""".toByteArray())
        val payload = base64Url("""{"sub":"$user","iat":${System.currentTimeMillis() / 1000}}""".toByteArray())
        val signature = base64Url(ByteArray(32).also(random::nextBytes))
        return "$header.$payload.$signature"
    }

    private fun base64Url(bytes: ByteArray): String =
        java.util.Base64.getUrlEncoder().withoutPadding().encodeToString(bytes)
}
