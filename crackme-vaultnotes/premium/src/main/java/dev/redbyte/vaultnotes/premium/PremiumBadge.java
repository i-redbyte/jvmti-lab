package dev.redbyte.vaultnotes.premium;

final class PremiumBadge {

    private PremiumBadge() {
    }

    static String text() {
        return PremiumLicense.isActive() ? "_Экспортировано с VaultNotes Premium_" : "_VaultNotes_";
    }
}
