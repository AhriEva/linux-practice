# Вариант 18 — эмулятор оболочки

C++17, Qt 6 Widgets. Этап 1: графический REPL, аргументы в кавычках,
команды-заглушки `ls`, `cd`, команда `exit`, диагностика ошибок.
Заголовок содержит настоящие имя пользователя и имя компьютера.

Сборка: `cmake -S . -B build && cmake --build build`.
Запуск: `./run.sh`. Тесты: `ctest --test-dir build --output-on-failure`.

Демонстрация: `ls "a b"`, `cd /tmp`, `unknown`, `exit extra`, `exit`.
