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

# lab1 / task2: добавляется по тому же образцу
# add_executable(lab1_task2 lab1/task2/main.cpp ...)
# target_include_directories(lab1_task2 PRIVATE lab1/task2/include)