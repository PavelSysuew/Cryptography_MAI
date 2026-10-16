# Минимальная версия CMake, которая нужна для этого файла
cmake_minimum_required(VERSION 3.10)

# Название проекта и язык (C++)
project(cryptography_mai CXX)

# Используем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Файл compile_commands.json для подсказок в VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Включаем предупреждения компилятора для всех программ ниже
add_compile_options(-Wall -Wextra)

# Папка с заголовками (.h): все программы ищут #include здесь
include_directories(lab1/include)


# ===== Программа lab1 (демо) =====
# Собирается из main.cpp и всех .cpp из src
add_executable(lab1
    lab1/main.cpp
    
    lab1/src/bitsPermutation.cpp
    lab1/src/FeistelNetwork.cpp
)


# ===== Тесты =====
# Отдельная программа с тестами: тот же src + файл тестов
add_executable(test_bitsPermutation
    lab1/tests/task1/test_bitsPermutation.cpp
    lab1/src/bitsPermutation.cpp
)
# Папка с общими функциями для тестов (testUtils.h)
target_include_directories(test_bitsPermutation PRIVATE lab1/tests)

# Включаем команду ctest
enable_testing()

# Каждый тест = запуск программы с номером теста.
# Тесты 1..20 из test_bitsPermutation.cpp: имя bitsPermutation_testN, запуск "test_bitsPermutation N"
foreach(n RANGE 1 20)
    add_test(NAME bitsPermutation_test${n} COMMAND test_bitsPermutation ${n})
endforeach()
