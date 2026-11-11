# part4-hidden

Агент части 4: что приложение прячет (флаги 10–12).

- `ClassFileLoadHook`: дамп DEX, загруженного из памяти, для jadx.
- Перехват результата native-метода на стороне Java.
- `ForceEarlyReturn` / `RetransformClasses`: клиентские проверки возвращают `true`.
