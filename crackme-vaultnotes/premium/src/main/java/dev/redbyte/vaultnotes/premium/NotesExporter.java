package dev.redbyte.vaultnotes.premium;

import java.util.List;

/**
 * Премиум-функция VaultNotes: экспорт заметок в Markdown.
 * Приложение находит этот класс по имени после расшифровки модуля.
 */
public final class NotesExporter {

    private NotesExporter() {
    }

    public static String export(List<String> notes) {
        StringBuilder out = new StringBuilder("# VaultNotes\n\n");
        for (int i = 0; i < notes.size(); i++) {
            out.append(i + 1).append(". ").append(MarkdownEscaper.escape(notes.get(i))).append('\n');
        }
        out.append('\n').append(PremiumBadge.text());
        return out.toString();
    }
}
