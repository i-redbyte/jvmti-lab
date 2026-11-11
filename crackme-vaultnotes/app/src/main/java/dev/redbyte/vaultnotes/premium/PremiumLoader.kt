package dev.redbyte.vaultnotes.premium

import android.content.Context
import dalvik.system.InMemoryDexClassLoader
import dev.redbyte.vaultnotes.vault.VaultNative
import java.nio.ByteBuffer
import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

/**
 * Премиум-модуль лежит в assets зашифрованным и расшифровывается только в памяти:
 * на диске его DEX никогда не появляется.
 */
class PremiumLoader(private val context: Context) {

    private var loader: ClassLoader? = null

    fun exportNotes(notes: List<String>): String {
        val exporter = classLoader().loadClass("dev.redbyte.vaultnotes.premium.NotesExporter")
        val export = exporter.getMethod("export", List::class.java)
        return export.invoke(null, notes) as String
    }

    private fun classLoader(): ClassLoader = loader ?: run {
        val blob = context.assets.open("premium.bin").use { it.readBytes() }
        // Старые сборки модуля шифровались прежним ключом: пробуем оба.
        val dex = runCatching { decrypt(blob, VaultNative.premiumKey()) }
            .getOrElse { decrypt(blob, LEGACY_PREMIUM_KEY.hexToByteArray()) }
        InMemoryDexClassLoader(ByteBuffer.wrap(dex), context.classLoader).also { loader = it }
    }

    private fun decrypt(blob: ByteArray, key: ByteArray): ByteArray {
        val cipher = Cipher.getInstance("AES/CBC/PKCS5Padding")
        cipher.init(Cipher.DECRYPT_MODE, SecretKeySpec(key, "AES"), IvParameterSpec(blob, 0, 16))
        return cipher.doFinal(blob, 16, blob.size - 16)
    }

    private companion object {
        // TODO выпилить после миграции всех пользователей на новый модуль.
        const val LEGACY_PREMIUM_KEY = "cb07e92e060fe3d89c85d39e11c56c06"
    }
}
