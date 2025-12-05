cmake_minimum_required(VERSION 3.16)
project(serwis_samochodow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GŁÓWNY PROGRAM
add_executable(serwis_samochodow
        main.cpp
        model.cpp
        ipc.cpp
)

# TEST JEDNOSTKOWY
add_executable(test_model
        tests/test_model.cpp
        model.cpp
)
