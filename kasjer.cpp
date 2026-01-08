// kasjer.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"

int main() {
    std::cout << "[kasjer] start" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[kasjer] blad ipc_init" << std::endl;
        return EXIT_FAILURE;
    }

    const int MAKS_RAPORTOW = 500;

    for (int i = 0; i < MAKS_RAPORTOW; ++i) {
        int id_klienta = 0;
        int rzeczywisty_czas = 0;
        int koszt_koncowy = 0;
        int stanowisko_id = 0;
        Samochod s{};

        std::cout << "[kasjer] czekam na raport (iteracja " << (i + 1) << ")" << std::endl;

        if (serwis_ipc_odbierz_raport(id_klienta, rzeczywisty_czas, koszt_koncowy, stanowisko_id, s) != SERWIS_IPC_OK) {
            std::cerr << "[kasjer] blad odbioru raportu" << std::endl;
            break;
        }

        std::cout << "[kasjer] raport: id_klienta=" << id_klienta
                  << ", stanowisko=" << stanowisko_id
                  << ", marka=" << s.marka
                  << ", czas=" << rzeczywisty_czas
                  << ", koszt=" << koszt_koncowy
                  << " -> platnosc OK, wydanie kluczykow"
                  << std::endl;
    }

    serwis_ipc_cleanup();
    std::cout << "[kasjer] koniec" << std::endl;
    return EXIT_SUCCESS;
}
