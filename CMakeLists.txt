cmake_minimum_required(VERSION 3.16)
project(serwis_samochodow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ------------------ Wspolna biblioteka (model + IPC + status stanowisk) ------------------
add_library(serwis_core
        model.cpp
        serwis_ipc.cpp
        stanowiska_status.cpp
        logger.cpp
)

target_include_directories(serwis_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ------------------ Program glowny ------------------
add_executable(serwis_samochodow
        main.cpp
)
target_link_libraries(serwis_samochodow PRIVATE serwis_core)

# ------------------ Procesy ------------------
add_executable(kierowca
        kierowca.cpp
)
target_link_libraries(kierowca PRIVATE serwis_core)

add_executable(pracownik_serwisu
        pracownik_serwisu.cpp
)
target_link_libraries(pracownik_serwisu PRIVATE serwis_core)

add_executable(mechanik
        mechanik.cpp
)
target_link_libraries(mechanik PRIVATE serwis_core)

add_executable(kasjer
        kasjer.cpp
)
target_link_libraries(kasjer PRIVATE serwis_core)

add_executable(kierownik
        kierownik.cpp
        stanowiska_status.cpp
)
target_include_directories(kierownik PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ------------------ Testy jednostkowe ------------------
add_executable(test_model
        tests/test_model.cpp
        model.cpp
)

target_include_directories(test_model PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
