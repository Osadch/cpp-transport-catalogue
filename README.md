# CityRouter
Реализация транспортного справочника. Строит карту маршрутов и получает данные по маршрутам и остановкам не её основе.
Получение и сохранение данных в формате JSON с использованием собственной библиотеки.
Визуализация карты маршрутов в формате SVG с использованием собственной библиотеки.
Поиск кратчайшего пути по заданным условиям на основе построенной карты маршрутов

# Использование:
1. Установка и настройка всех требуемых компонентов в среде разработки для запуска приложения
2. Вариант использования показан в main.cpp
3. "make_base": запрос на создание транспортного каталога
4. "process_requests: запрос на получение любой информации по остановкам, автобусам, оптимальным маршрутам

# Системные требования:
1. С++17 (STL)
2. GCC (MinGW-w64) 11.2.0

# Планы по доработке:
1. Добавить карту города со спутника
2. Протестировать реальные координаты и остановки
3. Создать десктопное приложение

# Стек технологий:
1. CMake 3.22.0
2. Protobyf-cpp 3.18.1
