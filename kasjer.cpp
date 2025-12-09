// kasjer.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"

int main() {
    std::cout << "[kasjer] start procesu kasjera" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[kasjer] blad inicjalizacji IPC" << std::endl;
        return EXIT_FAILURE;
    }

    const int LICZBA_RAPORTOW_DO_OBSLUGI = 5;

    for (int i = 0; i < LICZBA_RAPORTOW_DO_OBSLUGI; ++i) {
        int id_klienta = 0;
        int rzeczywisty_czas = 0;
        int koszt_koncowy = 0;
        Samochod s{};

        std::cout << "[kasjer] czekam na raport ("
                  << (i + 1) << "/" << LICZBA_RAPORTOW_DO_OBSLUGI << ")"
                  << std::endl;

        if (serwis_ipc_odbierz_raport(id_klienta, rzeczywisty_czas, koszt_koncowy, s)
            != SERWIS_IPC_OK) {
            std::cerr << "[kasjer] blad odbierania raportu" << std::endl;
            break;
            }

        std::cout << "[kasjer] otrzymano raport: "
                  << "id_klienta = " << id_klienta
                  << ", marka = " << s.marka
                  << ", rzeczywisty_czas = " << rzeczywisty_czas
                  << ", koszt_koncowy = " << koszt_koncowy
                  << std::endl;

        // TODO:
        //  - tutaj w przyszlosci mozna aktualizowac statystyki, licznik przyjetych itp.
    }

    serwis_ipc_cleanup();

    std::cout << "[kasjer] koniec procesu kasjera" << std::endl;
    return EXIT_SUCCESS;
}
