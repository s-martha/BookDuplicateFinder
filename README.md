# BookMatch

BookMatch ищет одинаковые и похожие текстовые документы, не изменяя исходные файлы. Это ранний вертикальный прототип: он рекурсивно читает только UTF-8 TXT-файлы и сравнивает все пары документов полным перебором.

## Сборка и тесты

Требуются CMake 3.22+, компилятор с C++20, OpenSSL и GoogleTest.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Использование

```bash
./build/bookmatch index /path/to/library --threshold 0.80
./build/bookmatch compare first.txt second.txt
```

`index` печатает совпадающие пары и сводку текущего запуска. `compare` показывает число токенов, класс совпадения, Jaccard по 5-словным шинглам и отношение длин. Порог `0.80` — стартовая настройка, а не гарантия дубликата.

Классы: `identical-file` (совпадает SHA-256 файла), `identical-normalized-text`, `likely-duplicate`, `partial-match` и `no-match`.

## Ограничения текущего этапа

Пока нет постоянного SQLite-индекса, LSH, CSV, многопоточности и извлечения EPUB/PDF/FB2/DOCX/DJVU/MOBI. Эти возможности будут добавляться последующими этапами из [PROJECT_SPEC.md](PROJECT_SPEC.md). Базовая нормализация обрабатывает латиницу и кириллицу, пробелы, пунктуацию и опциональную замену `ё` на `е`; полная Unicode-нормализация потребует отдельной обоснованной зависимости.
