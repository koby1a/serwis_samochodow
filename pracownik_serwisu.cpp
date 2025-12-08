// pracownik_serwisu.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"

int main() {
    std::cout << "[pracownik_serwisu] start procesu pracownika serwisu" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[pracownik_serwisu] blad inicjalizacji IPC" << std::endl;
        return EXIT_FAILURE;
    }

    const int LICZBA_ZGLOSZEN_DO_OBSLUGI = 5;
    int nastepne_id_klienta = 1;

    for (int i = 0; i < LICZBA_ZGLOSZEN_DO_OBSLUGI; ++i) {
        Samochod s{};
        std::cout << "[pracownik_serwisu] czekam na zgloszenie ("
                  << (i + 1) << "/" << LICZBA_ZGLOSZEN_DO_OBSLUGI << ")"
                  << std::endl;

        if (serwis_ipc_odbierz_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad odbierania zgloszenia" << std::endl;
            break;
        }

        std::cout << "[pracownik_serwisu] otrzymano zgloszenie: "
                  << "marka = " << s.marka
                  << ", czas_przyjazdu = " << s.czas_przyjazdu
                  << ", czas_naprawy = " << s.czas_naprawy
                  << ", krytyczna = " << s.krytyczna
                  << std::endl;

        // Sprawdzamy, czy marka jest obslugiwana
        if (!serwis_czy_marka_obslugiwana(s.marka)) {
            std::cout << "[pracownik_serwisu] auto odrzucone: nieobslugiwana marka "
                      << s.marka << std::endl;
            continue;
        }

        // Prosty model: czas_podstawowy = s.czas_naprawy,
        // czas_dodatkowy = 5 jesli usterka krytyczna, inaczej 0
        int czas_podstawowy = s.czas_naprawy;
        int czas_dodatkowy = (s.krytyczna ? 5 : 0);

        int przewidywany_czas = serwis_oblicz_czas_naprawy(
                czas_podstawowy,
                czas_dodatkowy,
                SERWIS_TRYB_NORMALNY
        );

        int id_klienta = nastepne_id_klienta++;

        std::cout << "[pracownik_serwisu] tworzę zlecenie: "
                  << "id_klienta = " << id_klienta
                  << ", przewidywany_czas = " << przewidywany_czas
                  << std::endl;

        if (serwis_ipc_wyslij_zlecenie(s, przewidywany_czas, id_klienta) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad wysylania zlecenia" << std::endl;
        }
    }

    serwis_ipc_cleanup();

    std::cout << "[pracownik_serwisu] koniec procesu pracownika serwisu" << std::endl;
    return EXIT_SUCCESS;
}
