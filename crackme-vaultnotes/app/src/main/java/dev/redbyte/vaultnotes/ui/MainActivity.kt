package dev.redbyte.vaultnotes.ui

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawingPadding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import dev.redbyte.vaultnotes.notes.NotesRepository
import dev.redbyte.vaultnotes.premium.PremiumLoader
import dev.redbyte.vaultnotes.vault.VaultNative

class MainActivity : ComponentActivity() {

    private val notes = NotesRepository()
    private val premium by lazy { PremiumLoader(applicationContext) }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme {
                Surface(Modifier.fillMaxSize()) {
                    var unlocked by remember { mutableStateOf(false) }
                    if (unlocked) {
                        NotesScreen(notes, onExport = { premium.exportNotes(notes.all()) })
                    } else {
                        LockScreen(onUnlock = { pin -> VaultNative.checkPin(pin).also { unlocked = it } })
                    }
                }
            }
        }
    }
}

@Composable
private fun LockScreen(onUnlock: (String) -> Boolean) {
    var pin by remember { mutableStateOf("") }
    var error by remember { mutableStateOf(false) }
    Column(
        Modifier.fillMaxSize().safeDrawingPadding().padding(24.dp),
        verticalArrangement = Arrangement.Center,
    ) {
        Text("VaultNotes", style = MaterialTheme.typography.headlineMedium)
        Text("Введите PIN, чтобы открыть заметки")
        OutlinedTextField(
            value = pin,
            onValueChange = { pin = it.filter(Char::isDigit).take(8); error = false },
            label = { Text("PIN") },
            isError = error,
            visualTransformation = PasswordVisualTransformation(),
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.NumberPassword),
            modifier = Modifier.fillMaxWidth().padding(top = 16.dp),
        )
        if (error) Text("Неверный PIN", color = MaterialTheme.colorScheme.error)
        Button(onClick = { error = !onUnlock(pin) }, modifier = Modifier.padding(top = 16.dp)) {
            Text("Открыть")
        }
    }
}

@Composable
private fun NotesScreen(repository: NotesRepository, onExport: () -> String) {
    var notes by remember { mutableStateOf(repository.all()) }
    var draft by remember { mutableStateOf("") }
    var exported by remember { mutableStateOf<String?>(null) }

    Column(Modifier.fillMaxSize().safeDrawingPadding().padding(16.dp)) {
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
            Text("Заметки", style = MaterialTheme.typography.headlineSmall)
            TextButton(onClick = { exported = onExport() }) { Text("Экспорт ★") }
        }
        Row(Modifier.fillMaxWidth().padding(vertical = 8.dp)) {
            OutlinedTextField(
                value = draft,
                onValueChange = { draft = it },
                label = { Text("Новая заметка") },
                modifier = Modifier.weight(1f),
            )
            Button(
                onClick = { repository.add(draft); draft = ""; notes = repository.all() },
                modifier = Modifier.padding(start = 8.dp, top = 8.dp),
            ) { Text("+") }
        }
        LazyColumn(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            items(notes) { note ->
                Card(Modifier.fillMaxWidth()) { Text(note, Modifier.padding(16.dp)) }
            }
        }
    }

    exported?.let { text ->
        AlertDialog(
            onDismissRequest = { exported = null },
            confirmButton = { TextButton(onClick = { exported = null }) { Text("Готово") } },
            title = { Text("Экспорт (премиум)") },
            text = { Text(text) },
        )
    }
}
