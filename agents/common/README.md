# common

Общий каркас агентов: RAII-обёртки для памяти JVMTI (`Deallocate`) и JNI-ссылок, работа с колбэками на чужих потоках (`AttachCurrentThread`, raw monitors), логирование через `__android_log_print`, разбор опций агента и запись JSON-отчёта.
