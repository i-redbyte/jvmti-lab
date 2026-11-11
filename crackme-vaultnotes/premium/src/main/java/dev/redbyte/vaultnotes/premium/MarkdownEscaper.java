package dev.redbyte.vaultnotes.premium;

final class MarkdownEscaper {

    private MarkdownEscaper() {
    }

    static String escape(String text) {
        return text.replace("*", "\\*").replace("_", "\\_").replace("#", "\\#");
    }
}
