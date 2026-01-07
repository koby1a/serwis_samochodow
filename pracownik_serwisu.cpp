// pracownik_serwisu.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"

static int serwis_wybierz_docelowe_stanowisko(char marka, unsigned int* seed) {
    // Normalizacja do wielkiej litery
    if (marka >= 'a' && marka <= 'z') marka = static_cast<char>(marka - 'a' + 'A');

    // U i Y: czasem na 8, czasem na 1..7
    if (marka == 'U' || marka == 'Y') {
        int r = serwis_losuj_int(seed, 0, 99);
        if (r < 40) { // 40% przypadkow na 8
            return 8;
        }
    }

    // Round-robin po 1..7
    static int rr = 1;
    int out = rr;
    rr++;
    if (rr > 7) rr = 1;
    return out;
}

int main() {
    std::cout << "[pracownik_serwisu] start" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[pracownik_serwisu] blad ipc_init" << std::endl;
        return EXIT_FAILURE;
    }

    unsigned int seed = 12345u;
    int nastepne_id_klienta = 1;

    const int MAKS_ZGLOSZEN = 500;

    for (int i = 0; i < MAKS_ZGLOSZEN; ++i) {
        Samochod s{};
        std::cout << "[pracownik_serwisu] czekam na zgloszenie" << std::endl;

        if (serwis_ipc_odbierz_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad odbioru zgloszenia" << std::endl;
            break;
        }

        std::cout << "[pracownik_serwisu] zgloszenie: marka=" << s.marka
                  << ", krytyczna=" << s.krytyczna
                  << std::endl;

        if (!serwis_czy_marka_obslugiwana(s.marka)) {
            std::cout << "[pracownik_serwisu] marka nieobslugiwana -> klient odjezdza" << std::endl;
            continue;
        }

        OfertaNaprawy oferta{};
        int ok_oferta = serwis_utworz_oferte(&oferta, &seed, 2, 5, 0, SERWIS_TRYB_NORMALNY);
        if (!ok_oferta) {
            std::cout << "[pracownik_serwisu] nie udalo sie utworzyc oferty" << std::endl;
            continue;
        }

        // 2% klientow odrzuca warunki
        int los = serwis_losuj_int(&seed, 0, 99);
        if (!serwis_klient_akceptuje_warunki(los, 2)) {
            std::cout << "[pracownik_serwisu] klient odrzucil oferte -> odjezdza" << std::endl;
            continue;
        }

        int id_klienta = nastepne_id_klienta++;
        int stanowisko_id = serwis_wybierz_docelowe_stanowisko(s.marka, &seed);

        std::cout << "[pracownik_serwisu] oferta OK: id_klienta=" << id_klienta
                  << ", koszt=" << oferta.koszt
                  << ", czas=" << oferta.czas
                  << ", stanowisko=" << stanowisko_id
                  << std::endl;

        if (serwis_ipc_wyslij_zlecenie(s, id_klienta, stanowisko_id, oferta) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad wysylki zlecenia" << std::endl;
            break;
        }
    }

    serwis_ipc_cleanup();
    std::cout << "[pracownik_serwisu] koniec" << std::endl;
    return EXIT_SUCCESS;
}
