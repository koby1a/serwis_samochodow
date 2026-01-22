cmake_minimum_required(VERSION 3.16)
project(serwis_samochodow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# Wspolne pliki (logika + IPC + UI)
set(SERWIS_CORE
        model.cpp
        serwis_ipc.cpp
        logger.cpp
        ui.cpp
)

# Program glowny (starter - uruchamia inne procesy)
add_executable(serwis_samochodow
        main.cpp
        ${SERWIS_CORE}
)

# Procesy
add_executable(kierowca
        kierowca.cpp
        ${SERWIS_CORE}
)

add_executable(pracownik_serwisu
        pracownik_serwisu.cpp
        ${SERWIS_CORE}
)

add_executable(mechanik
        mechanik.cpp
        ${SERWIS_CORE}
)

add_executable(kasjer
        kasjer.cpp
        ${SERWIS_CORE}
)

add_executable(kierownik
        kierownik.cpp
        ${SERWIS_CORE}
)

add_executable(dashboard
        dashboard.cpp
        ${SERWIS_CORE}
)

# Testy jednostkowe
add_executable(test_model
        tests/test_model.cpp
        ${SERWIS_CORE}
)
