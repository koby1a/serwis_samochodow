cmake_minimum_required(VERSION 3.16)
project(serwis_samochodow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

<<<<<<< HEAD
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(serwis_core
        src/model.cpp
        src/ipc.cpp
        src/logger.cpp
        src/ui.cpp
)

add_executable(serwis_samochodow apps/serwis_samochodow.cpp)
target_link_libraries(serwis_samochodow PRIVATE serwis_core)

add_executable(kierowca apps/kierowca.cpp)
target_link_libraries(kierowca PRIVATE serwis_core)

add_executable(pracownik_serwisu apps/pracownik_serwisu.cpp)
target_link_libraries(pracownik_serwisu PRIVATE serwis_core)

add_executable(mechanik apps/mechanik.cpp)
target_link_libraries(mechanik PRIVATE serwis_core)

add_executable(kasjer apps/kasjer.cpp)
target_link_libraries(kasjer PRIVATE serwis_core)

add_executable(kierownik apps/kierownik.cpp)
target_link_libraries(kierownik PRIVATE serwis_core)

add_executable(dashboard apps/dashboard.cpp)
target_link_libraries(dashboard PRIVATE serwis_core)

add_executable(test_model tests/test_model.cpp)
target_link_libraries(test_model PRIVATE serwis_core)
=======
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
>>>>>>> 49aa6d4 (v20)
