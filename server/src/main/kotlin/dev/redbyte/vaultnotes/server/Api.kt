package dev.redbyte.vaultnotes.server

import io.ktor.http.ContentType
import io.ktor.http.HttpStatusCode
import io.ktor.server.application.Application
import io.ktor.server.application.ApplicationCall
import io.ktor.server.request.receiveText
import io.ktor.server.response.respondText
import io.ktor.server.routing.get
import io.ktor.server.routing.post
import io.ktor.server.routing.routing
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.buildJsonArray
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.put

fun Application.vaultNotesApi(store: Store) {
    routing {
        get("/") {
            call.respondText("VaultNotes server is running")
        }

        post("/auth/login") {
            val body = call.receiveJson()
            val token = store.login(body.string("user"), body.string("password"))
            if (token == null) {
                call.respondJson(buildJsonObject { put("error", "invalid credentials") }, HttpStatusCode.Unauthorized)
            } else {
                call.respondJson(buildJsonObject {
                    put("token", token)
                    put("user", body.string("user"))
                    put("expiresIn", 3600)
                })
            }
        }

        post("/auth/logout") {
            call.bearer()?.let(store::logout)
            call.respondJson(buildJsonObject { put("ok", true) })
        }

        get("/notes") {
            val user = call.authorizedUser(store) ?: return@get
            call.respondJson(buildJsonObject {
                put("notes", buildJsonArray { store.notes(user).forEach { add(kotlinx.serialization.json.JsonPrimitive(it)) } })
            })
        }

        post("/notes") {
            val user = call.authorizedUser(store) ?: return@post
            store.addNote(user, call.receiveJson().string("text"))
            call.respondJson(buildJsonObject { put("ok", true) })
        }

        // «Оплата» премиума. Карта не проверяется, но ответ повторяет её номер целиком.
        post("/payments") {
            call.authorizedUser(store) ?: return@post
            val card = call.receiveJson().string("card").filter(Char::isDigit)
            call.respondJson(buildJsonObject {
                put("status", "approved")
                put("card", card)
                put("last4", card.takeLast(4))
                put("receiptId", "R-${System.currentTimeMillis()}")
            })
        }

        // Сервер знает, что премиума у пользователя нет. Спрашивает ли его клиент — другой вопрос.
        get("/premium/status") {
            call.authorizedUser(store) ?: return@get
            call.respondJson(buildJsonObject { put("premium", false) })
        }

        // Сюда шлёт события встроенный SDK аналитики. Сервер их просто принимает.
        post("/analytics") {
            call.receiveText()
            call.respondJson(buildJsonObject { put("ok", true) })
        }
    }
}

private suspend fun ApplicationCall.receiveJson(): JsonObject =
    runCatching { Json.parseToJsonElement(receiveText()) as JsonObject }.getOrDefault(JsonObject(emptyMap()))

private fun JsonObject.string(key: String): String = this[key]?.jsonPrimitive?.content.orEmpty()

private suspend fun ApplicationCall.respondJson(json: JsonObject, status: HttpStatusCode = HttpStatusCode.OK) =
    respondText(json.toString(), ContentType.Application.Json, status)

private fun ApplicationCall.bearer(): String? =
    request.headers["Authorization"]?.removePrefix("Bearer ")?.trim()

// Пользователь по токену из заголовка Authorization. Нет пользователя — сразу 401.
private suspend fun ApplicationCall.authorizedUser(store: Store): String? {
    val user = store.userOf(bearer())
    if (user == null) respondJson(buildJsonObject { put("error", "unauthorized") }, HttpStatusCode.Unauthorized)
    return user
}
