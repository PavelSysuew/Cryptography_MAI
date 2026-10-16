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
    lab1/src/bytesUtils.cpp
    lab1/src/DesKeyExpansion.cpp
    lab1/src/DesRoundFunction.cpp
    lab1/src/DES.cpp
    lab1/src/padding.cpp
    lab1/src/CipherContext.cpp
    lab1/demo/demo.cpp
)
# Демо (задание 5): заголовок demo.h и путь к корню проекта (файлы по умолчанию, папка demo_output)
target_include_directories(lab1 PRIVATE lab1/demo)
target_compile_definitions(lab1 PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")


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


# ===== Тесты задания 3: сеть Фейстеля =====
add_executable(test_FeistelNetwork
    lab1/tests/task3/test_FeistelNetwork.cpp
    lab1/src/FeistelNetwork.cpp
    lab1/src/bytesUtils.cpp
)
target_include_directories(test_FeistelNetwork PRIVATE lab1/tests)

# Тесты 1..10 из test_FeistelNetwork.cpp: имя FeistelNetwork_testN, запуск "test_FeistelNetwork N"
foreach(n RANGE 1 10)
    add_test(NAME FeistelNetwork_test${n} COMMAND test_FeistelNetwork ${n})
endforeach()


# ===== Тесты задания 4: DES =====
add_executable(test_DES
    lab1/tests/task4/test_DES.cpp
    lab1/src/DES.cpp
    lab1/src/DesKeyExpansion.cpp
    lab1/src/DesRoundFunction.cpp
    lab1/src/FeistelNetwork.cpp
    lab1/src/bitsPermutation.cpp
    lab1/src/bytesUtils.cpp
)
target_include_directories(test_DES PRIVATE lab1/tests)

# Тесты 1..13 из test_DES.cpp: имя DES_testN, запуск "test_DES N"
foreach(n RANGE 1 13)
    add_test(NAME DES_test${n} COMMAND test_DES ${n})
endforeach()


# ===== Тесты задания 2.4, шаг 1: набивки =====
add_executable(test_padding
    lab1/tests/task2/test_padding.cpp
    lab1/src/padding.cpp
)
target_include_directories(test_padding PRIVATE lab1/tests)

# Тесты 1..17 из test_padding.cpp: имя padding_testN, запуск "test_padding N"
foreach(n RANGE 1 17)
    add_test(NAME padding_test${n} COMMAND test_padding ${n})
endforeach()


# ===== Тесты задания 2.4, шаг 2: контекст и режимы =====
add_executable(test_CipherContext
    lab1/tests/task2/test_CipherContext.cpp
    lab1/src/CipherContext.cpp
    lab1/src/padding.cpp
    lab1/src/DES.cpp
    lab1/src/DesKeyExpansion.cpp
    lab1/src/DesRoundFunction.cpp
    lab1/src/FeistelNetwork.cpp
    lab1/src/bitsPermutation.cpp
    lab1/src/bytesUtils.cpp
)
target_include_directories(test_CipherContext PRIVATE lab1/tests)

# Тесты 1..31 из test_CipherContext.cpp: имя CipherContext_testN, запуск "test_CipherContext N"
foreach(n RANGE 1 31)
    add_test(NAME CipherContext_test${n} COMMAND test_CipherContext ${n})
endforeach()
