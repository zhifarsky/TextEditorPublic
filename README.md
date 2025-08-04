# Текстовый редактор
- В основе структура данных [Piece Table](https://en.wikipedia.org/wiki/Piece_table)
- Управление памяью при помощи стратегии [Arena](https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator)
- Собирается без стандартных библиотек C/C++, не требует зависимостей кроме [ImGui](https://github.com/ocornut/imgui), включенного в проект
- Реализованы самописные аналоги стандартных функций
- Поддерживает UTF-8
- Интерфейс на ImGui

## Демонстрация
<img src="https://github.com/user-attachments/assets/a4fecb59-7cb4-4a90-a594-f29ba0c1c511" width="600"/>

## Сборка

### Требования:
- Должен быть установлен MSVC

### Шаги:
1. Запустить buildLibs.bat
2. Запустить build.bat (для Release сборки запускать с аргументом "RELEASE")
