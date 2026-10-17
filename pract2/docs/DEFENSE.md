# Демонстрация варианта 18, практика 2

## Запуск

В каталоге pract2 выполните `./scripts/demo.sh`. Покажите описание PlantUML,
дерево общей зависимости и порядок `E -> D -> B -> C -> A`.
Граф строится от A к его зависимостям, порядок загрузки идёт в обратном направлении.

## Глубина и фильтр

```sh
./build/dependencies --package A --repository examples/simple.json --test-mode --ascii --max-depth 0
./build/dependencies --package A --repository examples/simple.json --test-mode --ascii --max-depth 1
./build/dependencies --package A --repository examples/simple.json --test-mode --ascii --exclude B
```

При глубине 0 остаётся A. При глубине 1 видны A/B/C, но D/E не загружаются.
При исключении B исчезает также D, поскольку путь к нему проходил только через B.
На diamond.json исключение B оставляет D: к нему ещё можно пройти через C.

## Цикл

```sh
./build/dependencies --package A --repository examples/cycle.json --test-mode --ascii
./build/dependencies --package A --repository examples/cycle.json --test-mode --ascii --order
```

Первый запуск завершится нормально и отметит возврат в A как `[cycle]`.
Второй покажет граф, но сообщит об отсутствии полного топологического порядка
и вернёт 1. Цикл не приводит к бесконечному обходу.

## Ошибки параметров

```sh
./build/dependencies --package A --test-mode --repository missing.json
./build/dependencies --package itoa --repository not-url
./build/dependencies --package A --test-mode --repository examples/simple.json --max-depth -1
./build/dependencies --package A --test-mode --repository examples/simple.json --ascii=false
```

Диагностика выводится в stderr, код возврата равен 1. Аналогично проверяются
неверное имя пакета, неизвестная опция и исключение корня фильтром.

## Реальные пакеты

Выполните `./scripts/real-demo.sh`: itoa, getrandom, rand_chacha.
Покажите файлы examples/real/*.puml и сравнение в docs/COMPARISON.md.
Приложение не запускает Cargo: данные приходят по HTTP из sparse index.
Cargo вызывается только отдельным scripts/compare.sh как независимый эталон.

Для чистого PlantUML сохраните stdout собранного бинарника без --ascii/--order.
Это текстовая диаграмма для последующего открытия в средстве просмотра PlantUML.

## Код и история

Покажите очередь в buildGraph и отсутствие рекурсии в обходе.
При повторном запросе узла объединяются features; при их дополнении узел
повторно ставится в очередь. Для порядка используется алгоритм Кана.

`git log --oneline --decorate` показывает пять этапов.
`./scripts/check.sh` запускает все автоматические проверки без внешней сети.
