# Вариант 18: граф зависимостей Cargo

Практическая работа № 2 по конфигурационному управлению, ИКБО-13-25.
C++17, Qt 6 Core. Этап 1: CLI и проверка параметров.

Сборка: `cmake -S . -B build && cmake --build build --parallel`.
Запуск: `./run.sh --package getrandom@0.2.16`.
Тесты: `ctest --test-dir build --output-on-failure`.

Параметры: `--package`, `--repository`, `--test-mode`, `--ascii`,
`--max-depth`, `--exclude`. Значения выводятся как ключ=значение.

## Этап 2

Метаданные читаются собственным HTTP-клиентом из Cargo sparse index.
Индекс — JSON Lines; учитываются зависимости normal/build, default features,
переименования, версии SemVer и yanked. Dev-зависимости не включаются.
Cargo и библиотеки для анализа зависимостей не используются.
