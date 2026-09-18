cmake_minimum_required(VERSION 3.16)
project(Cryptography_MAI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

foreach(lab RANGE 1 4)
    foreach(task RANGE 1 2)
        set(target_name "lab${lab}_task${task}")
        add_executable(${target_name} "lab${lab}/task${task}/main.cpp")
    endforeach()
endforeach()
