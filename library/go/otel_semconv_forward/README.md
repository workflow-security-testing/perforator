# otel_semconv_forward

Пакет дает стабильный путь импорта для OpenTelemetry semantic conventions:

```go
import semconv "a.yandex-team.ru/library/go/otel_semconv_forward"
```

Цель - изолировать обновления semconv в одной директории и не переписывать импорты по всему монорепозиторию при каждом bump версии.

## Зачем нужен прокси

- Прямые импорты вида `go.opentelemetry.io/otel/semconv/v1.xx.0` требуют массового codemod при каждом обновлении semconv.
- Этот пакет реэкспортирует символы из одного upstream-пакета semconv и скрывает конкретный versioned import path от пользователей.

## Настройка версий

Единый источник правды - файл `versions.mk`:

- `OTEL_VERSION` - версия OpenTelemetry в корневом `go.mod` Arcadia (метаданные совместимости).
- `SEMCONV_VERSION` - версия semantic conventions.
- `SEMCONV_PKG` - вычисляемый import path, который используют генератор и проверка.

В директории нет собственного `go.mod`, используется корневой модуль Arcadia (`a.yandex-team.ru`).

## Как обновить semconv

Запустите:

```bash
make bump-semconv SEMCONV=v1.27.0
```

Команда:

1. обновляет `versions.mk`
2. проверяет, что `SEMCONV_PKG` доступен в текущем наборе зависимостей
3. заново генерирует `semconv_gen.go`
4. запускает тесты

По умолчанию `Makefile` использует `ya tool go`.

## Частые проблемы

- `failed to load upstream package`:
  - выбранная `SEMCONV_VERSION` отсутствует для текущей `OTEL_VERSION`;
  - увеличьте `OTEL_VERSION`.
- В CI найден устаревший сгенерированный файл:
  - выполните `make generate` и закоммитьте `semconv_gen.go`.

## Проверки CI

Запуск:

```bash
make ci
```

Проверяется:

- совместимость пакета semconv
- актуальность сгенерированного кода
- тесты

Дополнительно (тяжелый проход по монорепозиторию):

```bash
make verify-no-direct-semconv
```

## Примечание по совместимости

`SEMCONV_VERSION` не всегда доступна в той же версии `OTEL_VERSION`.
Известная базовая связка:

| SEMCONV_VERSION | Минимальная OTEL_VERSION |
| --- | --- |
| v1.26.0 | v1.37.0 |
