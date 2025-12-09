cmake_minimum_required(VERSION 3.16)
project(serwis_samochodow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Program glowny
add_executable(serwis_samochodow
        main.cpp
        model.cpp
        serwis_ipc.cpp
)

# Proces kierowcy (generator zgloszen)
add_executable(kierowca
        kierowca.cpp
        model.cpp
        serwis_ipc.cpp
        komunikaty.h
)

# Proces pracownika serwisu
add_executable(pracownik_serwisu
        pracownik_serwisu.cpp
        model.cpp
        serwis_ipc.cpp
        komunikaty.h
)

# Proces mechanika
add_executable(mechanik
        mechanik.cpp
        model.cpp
        serwis_ipc.cpp
        komunikaty.h
)

# Proces kasjera
add_executable(kasjer
        kasjer.cpp
        model.cpp
        serwis_ipc.cpp
        komunikaty.h
)

# Testy jednostkowe logiki
add_executable(test_model
        tests/test_model.cpp
        model.cpp
)
