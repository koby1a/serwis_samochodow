// kierowca.cpp
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "model.h"
#include "serwis_ipc.h"

int main() {
    std::cout << "[kierowca] proces uruchomiony" << std::endl;

    // Inicjalizacja IPC (utworzenie / otwarcie kolejki zgloszen)
    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[kierowca] blad inicjalizacji IPC" << std::endl;
        return 1;
    }

    // Prosta inicjalizacja generatora liczb losowych
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Przykladowo wysylamy 5 samochodow
    const int LICZBA_SAMOCHODOW = 5;

    for (int i = 0; i < LICZBA_SAMOCHODOW; ++i) {
        Samochod s{};

        // Losujemy marke z zakresu 'A'..'Z'
        s.marka = static_cast<char>('A' + (std::rand() % 26));
        s.czas_przyjazdu = i;       // na razie po prostu numer iteracji
        s.czas_naprawy = 10 + i;    // przykladowy czas
        s.krytyczna = (std::rand() % 2);  // 0 lub 1

        std::cout << "[kierowca] wysylam zgloszenie, marka = "
                  << s.marka
                  << ", krytyczna = " << s.krytyczna
                  << std::endl;

        if (serwis_ipc_wyslij_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[kierowca] blad wyslania zgloszenia" << std::endl;
            // Nie przerywamy od razu petli, ale mozna rozwazyc wyjscie
        }
    }

    std::cout << "[kierowca] proces konczy dzialanie" << std::endl;
    // UWAGA: serwis_ipc_cleanup docelowo wywoluje tylko proces glowny.
    // Tutaj go nie wywolujemy, zeby nie usuwac kolejki w procesie kierowcy.
    return 0;
}
