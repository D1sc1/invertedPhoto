# invertedPhoto

Многопоточная инверсия (негатив) изображений на C++ с логированием и CI/CD на
GitHub Actions.

Изображение разбивается на участки пикселей, каждый участок инвертируется в своём
потоке (`value -> 255 - value`), после чего все потоки сливаются обратно в главный
поток (`join`). Альфа-канал не трогается.

## Возможности

- **Многопоточность** — пиксели делятся на `N` равных чанков, по одному `std::thread`
  на чанк; затем все потоки джойнятся в главный поток.
- **Логи** — потокобезопасный логгер пишет в консоль и в файл, с таймстемпами,
  уровнями (`DEBUG/INFO/WARN/ERROR`) и id потоков.
- **Форматы** — чтение: PNG, JPG, BMP, TGA, PSD, GIF, HDR, PIC, PNM (через `stb_image`);
  запись: PNG, JPG, BMP, TGA (по расширению файла).
- **CI/CD** — GitHub Actions собирает проект на Linux и Windows, гоняет тест
  round-trip и выкладывает логи и картинки как артефакты.
- **Без внешних зависимостей** — `stb_image` лежит в `third_party/`.

## Структура

```
invertedPhoto/
├── main.cpp                  # CLI и оркестрация
├── include/                  # заголовки
│   ├── logger.hpp
│   ├── image.hpp
│   └── inverter.hpp
├── src/
│   ├── logger.cpp            # потокобезопасный логгер
│   ├── image.cpp             # загрузка/сохранение (stb)
│   ├── inverter.cpp          # многопоточная инверсия + join
│   └── stb_impl.cpp          # реализация stb (отдельный TU)
├── third_party/stb/          # stb_image.h, stb_image_write.h
├── scripts/
│   ├── make_sample.py        # генерация тестовой картинки (PPM)
│   └── verify_roundtrip.py   # тест корректности для CTest
├── .github/workflows/ci.yml  # GitHub Actions
└── CMakeLists.txt
```

## Сборка

Нужны CMake ≥ 3.15 и компилятор с C++17.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Бинарь:
- Linux/macOS: `build/inverted_photo`
- Windows (MSVC): `build/Release/inverted_photo.exe`

## Запуск

```bash
# сгенерировать тестовую картинку
python scripts/make_sample.py assets/sample.ppm 256

# инвертировать в 8 потоков с подробными логами
./build/inverted_photo assets/sample.ppm output/inverted.png --threads 8 --verbose --log logs/run.log
```

### Опции

```
inverted_photo <input> <output> [options]

  --threads N    число рабочих потоков (по умолчанию: все ядра CPU)
  --log FILE     файл лога (по умолчанию: invert.log)
  --verbose, -v  подробные (DEBUG) логи, включая диапазоны пикселей по потокам
  --quiet, -q    только предупреждения и ошибки
  --help, -h     справка
```

### Пример лога

```
[2026-06-28 19:30:00.123] [INFO ] [tid 0x1a2b] === inverted_photo started ===
[2026-06-28 19:30:00.130] [INFO ] [tid 0x1a2b] Loaded 256x256 image, channels=3
[2026-06-28 19:30:00.131] [INFO ] [tid 0x1a2b] Inverting 256x256 image (3 channels, 65536 px) using 8 thread(s)
[2026-06-28 19:30:00.131] [DEBUG] [tid 0x3c4d] worker #0 inverting pixels [0, 8192)  (8192 px)
[2026-06-28 19:30:00.132] [INFO ] [tid 0x1a2b] Spawned 8 worker thread(s); joining them back into the main thread...
[2026-06-28 19:30:00.133] [INFO ] [tid 0x1a2b] All workers joined. Inversion finished in 0.85 ms
[2026-06-28 19:30:00.140] [INFO ] [tid 0x1a2b] === inverted_photo finished ===
```

## Тесты

```bash
ctest --test-dir build --output-on-failure
```

Тест `roundtrip` проверяет, что `inv(inv(inv(x))) == inv(x)` на уровне пикселей:
картинка инвертируется три раза, и PNG после первой инверсии должен совпадать байт
в байт с PNG после третьей.

## CI/CD

`.github/workflows/ci.yml` на каждый push/PR в `main`:

1. собирает проект на `ubuntu-latest` и `windows-latest`;
2. запускает `ctest` (тест round-trip);
3. генерирует картинку и инвертирует её с подробными логами;
4. выкладывает `output/`, `logs/` и `assets/sample.ppm` как артефакты сборки.
