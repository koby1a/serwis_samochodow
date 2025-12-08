// mechanik.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"

int main() {
    std::cout << "[mechanik] start procesu mechanika" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[mechanik] blad inicjalizacji IPC" << std::endl;
        return EXIT_FAILURE;
    }

    const int LICZBA_ZLECEN_DO_OBSLUGI = 5;

    for (int i = 0; i < LICZBA_ZLECEN_DO_OBSLUGI; ++i) {
        Samochod s{};
        int przewidywany_czas = 0;
        int id_klienta = 0;

        std::cout << "[mechanik] czekam na zlecenie ("
                  << (i + 1) << "/" << LICZBA_ZLECEN_DO_OBSLUGI << ")"
                  << std::endl;

        if (serwis_ipc_odbierz_zlecenie(s, przewidywany_czas, id_klienta) != SERWIS_IPC_OK) {
            std::cerr << "[mechanik] blad odbierania zlecenia" << std::endl;
            break;
        }

        std::cout << "[mechanik] otrzymano zlecenie: "
                  << "id_klienta = " << id_klienta
                  << ", marka = " << s.marka
                  << ", przewidywany_czas = " << przewidywany_czas
                  << std::endl;

        // TODO:
        //  - na tym etapie mozemy zasymulowac naprawe (sleep)
        //  - pozniej wysylamy raport do kasjera (trzecia kolejka)
    }

    serwis_ipc_cleanup();

    std::cout << "[mechanik] koniec procesu mechanika" << std::endl;
    return EXIT_SUCCESS;
}
