package dev.redbyte.vaultnotes.vault

/**
 * Мост в libvault.so. Библиотека загружается при первом обращении к объекту,
 * то есть когда пользователь впервые вводит PIN.
 */
object VaultNative {

    init {
        System.loadLibrary("vault")
    }

    /** Проверка PIN. Реализация регистрируется в JNI_OnLoad через RegisterNatives. */
    external fun checkPin(pin: String): Boolean

    /** Ключ премиум-модуля. Связывается по имени символа Java_..._premiumKey. */
    external fun premiumKey(): ByteArray

    /** Токен синхронизации заметок с сервером. Тоже регистрируется в JNI_OnLoad. */
    external fun syncToken(): String
}
