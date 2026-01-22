cmake_minimum_required(VERSION 3.16)
project(serwis_samochodow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

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
