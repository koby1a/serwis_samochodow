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

    // Seed deterministyczny dla tego procesu
    unsigned int seed = 12345u;

    int nastepne_id_klienta = 1;

    const int MAKS_ZGLOSZEN = 200;

    for (int i = 0; i < MAKS_ZGLOSZEN; ++i) {
        Samochod s{};
        std::cout << "[pracownik_serwisu] czekam na zgloszenie (iteracja " << (i + 1) << ")"
                  << std::endl;

        if (serwis_ipc_odbierz_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad odbierania zgloszenia" << std::endl;
            break;
        }

        std::cout << "[pracownik_serwisu] odebrano zgloszenie: marka=" << s.marka
                  << ", krytyczna=" << s.krytyczna
                  << std::endl;

        // Sprawdzenie czy marka jest obslugiwana
        if (!serwis_czy_marka_obslugiwana(s.marka)) {
            std::cout << "[pracownik_serwisu] marka nieobslugiwana, klient odjezdza" << std::endl;
            continue;
        }

        // Tworzymy oferte (losowanie uslug + koszt + czas)
        OfertaNaprawy oferta{};
        int ok_oferta = serwis_utworz_oferte(&oferta,
                                             &seed,
                                             2, 5,               // min/max liczba uslug w ofercie
                                             0,                  // czas_dodatkowy na razie 0
                                             SERWIS_TRYB_NORMALNY);

        if (!ok_oferta) {
            std::cout << "[pracownik_serwisu] nie udalo sie utworzyc oferty" << std::endl;
            continue;
        }

        // 2% klientow odrzuca warunki (czas i koszt)
        int los = serwis_losuj_int(&seed, 0, 99);
        if (!serwis_klient_akceptuje_warunki(los, 2)) {
            std::cout << "[pracownik_serwisu] klient odrzucil oferte i odjechal" << std::endl;
            continue;
        }

        int id_klienta = nastepne_id_klienta++;
        std::cout << "[pracownik_serwisu] oferta zaakceptowana: id_klienta=" << id_klienta
                  << ", koszt=" << oferta.koszt
                  << ", czas=" << oferta.czas
                  << ", liczba_uslug=" << oferta.liczba_uslug
                  << std::endl;

        // Wysylamy zlecenie do mechanika (wraz z oferta)
        if (serwis_ipc_wyslij_zlecenie(s, id_klienta, oferta) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad wysylania zlecenia" << std::endl;
            break;
        }
    }

    serwis_ipc_cleanup();

    std::cout << "[pracownik_serwisu] koniec procesu pracownika serwisu" << std::endl;
    return EXIT_SUCCESS;
}
