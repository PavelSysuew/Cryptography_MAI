cmake_minimum_required(VERSION 3.10)
project(cryptography_mai CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # для подсказок в VS Code

# lab1 / task1: перестановка битов
add_executable(lab1_task1
    lab1/task1/main.cpp
    lab1/task1/src/bitsPermutation.cpp
)
target_include_directories(lab1_task1 PRIVATE lab1/task1/include)

if(MSVC)
    target_compile_options(lab1_task1 PRIVATE /W4)
else()
    target_compile_options(lab1_task1 PRIVATE -Wall -Wextra)
endif()

# lab1 / task1: тесты перестановки битов (ctest или ./build/lab1_task1_tests)
enable_testing()
add_executable(lab1_task1_tests
    lab1/task1/tests/test_bitsPermutation.cpp
    lab1/task1/src/bitsPermutation.cpp
)
target_include_directories(lab1_task1_tests PRIVATE lab1/task1/include)
# Каждый тест регистрируется отдельно (номер = аргумент запуска), порядок как в test_bitsPermutation.cpp
set(LAB1_TASK1_TEST_NAMES
    leftToRight_from_one
    leftToRight_from_zero
    rightToLeft_from_one
    rightToLeft_from_zero
    empty_pBlock
    nonexistent_bit
    compression
    expansion
    invalid_values
)
set(testNumber 1)
foreach(testName IN LISTS LAB1_TASK1_TEST_NAMES)
    add_test(NAME lab1_task1_${testNumber}_${testName} COMMAND lab1_task1_tests ${testNumber})
    math(EXPR testNumber "${testNumber} + 1")
endforeach()

if(MSVC)
    target_compile_options(lab1_task1_tests PRIVATE /W4)
else()
    target_compile_options(lab1_task1_tests PRIVATE -Wall -Wextra)
endif()

# lab1 / task2: добавляется по тому же образцу
# add_executable(lab1_task2 lab1/task2/main.cpp ...)
# target_include_directories(lab1_task2 PRIVATE lab1/task2/include)