// pracownik_serwisu.cpp
#include <iostream>
#include <cstdlib>
#include <string>
#include "model.h"
#include "serwis_ipc.h"
#include "stanowiska_status.h"
#include "logger.h"

// Godziny serwisu (minuty w dobie): 08:00-16:00
static const int TP = 8 * 60;
static const int TK = 16 * 60;

// Jezeli do otwarcia mniej niz T1, klient moze poczekac
static const int T1 = 60;

// K1/K2 (okienka rejestracji)
static const int K1 = 3;
static const int K2 = 5;

static std::string serwis_get_arg_str(int argc, char** argv, const std::string& key, const std::string& def) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == key) {
            return std::string(argv[i + 1]);
        }
    }
    return def;
}

static int serwis_czy_w_godzinach(int t) {
    return (t >= TP && t < TK) ? 1 : 0;
}

static int serwis_minuty_do_otwarcia(int t) {
    if (t < TP) return TP - t;
    if (t >= TK) return (1440 - t) + TP;
    return 0;
}

static int serwis_czy_moze_czekac_poza_godzinami(const Samochod& s) {
    if (s.krytyczna) return 1;
    int do_otwarcia = serwis_minuty_do_otwarcia(s.czas_przyjazdu);
    if (do_otwarcia < T1) return 1;
    return 0;
}

static int serwis_aktualizuj_okienka(int aktywne_okienka, int dl_kolejki) {
    if (aktywne_okienka < 1) aktywne_okienka = 1;
    if (aktywne_okienka > 3) aktywne_okienka = 3;

    if (dl_kolejki > K2) aktywne_okienka = 3;
    else if (dl_kolejki > K1 && aktywne_okienka < 2) aktywne_okienka = 2;

    if (aktywne_okienka >= 2 && dl_kolejki <= 2) aktywne_okienka = 1;
    if (aktywne_okienka == 3 && dl_kolejki <= 3) {
        aktywne_okienka = 2;
        if (dl_kolejki <= 2) aktywne_okienka = 1;
    }
    return aktywne_okienka;
}

static int serwis_stanowisko_dostepne(int id) {
    if (serwis_stanowisko_ma_prosbe_zamkniecia(id)) return 0;
    if (serwis_stanowisko_jest_zamkniete(id)) return 0;
    return 1;
}

static int serwis_wybierz_stanowisko_dla_marki(char marka, unsigned int* seed) {
    if (marka >= 'a' && marka <= 'z') marka = static_cast<char>(marka - 'a' + 'A');

    if (marka == 'U' || marka == 'Y') {
        int r = serwis_losuj_int(seed, 0, 99);
        if (r < 40 && serwis_stanowisko_dostepne(8)) {
            return 8;
        }
    }

    static int rr = 1;
    for (int probe = 0; probe < 7; ++probe) {
        int cand = rr;
        rr++;
        if (rr > 7) rr = 1;
        if (serwis_stanowisko_dostepne(cand)) return cand;
    }

    if ((marka == 'U' || marka == 'Y') && serwis_stanowisko_dostepne(8)) {
        return 8;
    }

    return -1;
}

int main(int argc, char** argv) {
    // Logger
    std::string log_path = serwis_get_arg_str(argc, argv, "--log", "raport_symulacji.log");
    serwis_logger_set_file(log_path.c_str());

    std::cout << "[pracownik_serwisu] start" << std::endl;
    serwis_log("pracownik", "start");

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[pracownik_serwisu] blad ipc_init" << std::endl;
        serwis_log("pracownik", "blad ipc_init");
        return EXIT_FAILURE;
    }

    serwis_logf("pracownik", "parametry Tp=%d Tk=%d T1=%d K1=%d K2=%d", TP, TK, T1, K1, K2);

    unsigned int seed = 12345u;
    int nastepne_id_klienta = 1;

    int aktywne_okienka = 1;
    int dl_kolejki = 0;

    const int MAKS_ZGLOSZEN = 500;

    for (int iter = 0; iter < MAKS_ZGLOSZEN; ++iter) {
        if (serwis_pozar_jest()) {
            std::cout << "[pracownik_serwisu] pozar.flag -> koniec" << std::endl;
            serwis_log("pracownik", "poz ar.flag -> koniec");
            break;
        }

        Samochod s{};
        std::cout << "[pracownik_serwisu] czekam na zgloszenie" << std::endl;
        serwis_log("pracownik", "czekam na zgloszenie");

        if (serwis_ipc_odbierz_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad odbioru zgloszenia" << std::endl;
            serwis_log("pracownik", "blad odbioru zgloszenia");
            break;
        }

        std::cout << "[pracownik_serwisu] zgloszenie: marka=" << s.marka
                  << ", czas_przyjazdu=" << s.czas_przyjazdu
                  << ", krytyczna=" << s.krytyczna
                  << std::endl;

        serwis_logf("pracownik", "zgloszenie marka=%c czas_przyjazdu=%d krytyczna=%d",
                    s.marka, s.czas_przyjazdu, s.krytyczna);

        // Godziny otwarcia
        if (!serwis_czy_w_godzinach(s.czas_przyjazdu)) {
            int do_otwarcia = serwis_minuty_do_otwarcia(s.czas_przyjazdu);

            if (!serwis_czy_moze_czekac_poza_godzinami(s)) {
                std::cout << "[pracownik_serwisu] poza godzinami, klient nie moze czekac (do_otwarcia="
                          << do_otwarcia << ") -> odjezdza" << std::endl;

                serwis_logf("pracownik", "odrzut poza_godzinami do_otwarcia=%d", do_otwarcia);
                continue;
            }

            std::cout << "[pracownik_serwisu] poza godzinami, klient czeka (do_otwarcia="
                      << do_otwarcia << ", krytyczna=" << s.krytyczna << ")" << std::endl;

            serwis_logf("pracownik", "klient czeka poza_godzinami do_otwarcia=%d krytyczna=%d",
                        do_otwarcia, s.krytyczna);
        }

        // Kolejka rejestracji + okienka
        dl_kolejki++;
        int stare = aktywne_okienka;
        aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
        if (stare != aktywne_okienka) {
            std::cout << "[pracownik_serwisu] zmiana okienek: " << stare
                      << " -> " << aktywne_okienka << " (kolejka=" << dl_kolejki << ")" << std::endl;
            serwis_logf("pracownik", "okienka %d -> %d (kolejka=%d)", stare, aktywne_okienka, dl_kolejki);
        }

        // Marka obslugiwana?
        if (!serwis_czy_marka_obslugiwana(s.marka)) {
            std::cout << "[pracownik_serwisu] marka nieobslugiwana -> klient odjezdza" << std::endl;
            serwis_logf("pracownik", "odrzut marka=%c nieobslugiwana", s.marka);

            dl_kolejki--;
            aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
            continue;
        }

        // Oferta
        OfertaNaprawy oferta{};
        if (!serwis_utworz_oferte(&oferta, &seed, 2, 5, 0, SERWIS_TRYB_NORMALNY)) {
            std::cout << "[pracownik_serwisu] nie udalo sie utworzyc oferty" << std::endl;
            serwis_log("pracownik", "blad utworzenia oferty");

            dl_kolejki--;
            aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
            continue;
        }

        // 2% odrzuca
        int los = serwis_losuj_int(&seed, 0, 99);
        if (!serwis_klient_akceptuje_warunki(los, 2)) {
            std::cout << "[pracownik_serwisu] klient odrzucil oferte -> odjezdza" << std::endl;
            serwis_logf("pracownik", "odrzut oferta (2%%) los=%d", los);

            dl_kolejki--;
            aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
            continue;
        }

        // Wybor stanowiska z uwzglednieniem zamknietych
        int stanowisko_id = serwis_wybierz_stanowisko_dla_marki(s.marka, &seed);
        if (stanowisko_id == -1) {
            std::cout << "[pracownik_serwisu] brak dostepnych stanowisk -> klient odjezdza" << std::endl;
            serwis_log("pracownik", "brak dostepnych stanowisk -> odrzut");

            dl_kolejki--;
            aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
            continue;
        }

        int id_klienta = nastepne_id_klienta++;

        std::cout << "[pracownik_serwisu] rejestracja OK: id_klienta=" << id_klienta
                  << ", stanowisko=" << stanowisko_id
                  << ", koszt=" << oferta.koszt
                  << ", czas=" << oferta.czas
                  << std::endl;

        serwis_logf("pracownik", "rejestracja OK id_klienta=%d stanowisko=%d marka=%c koszt=%d czas=%d",
                    id_klienta, stanowisko_id, s.marka, oferta.koszt, oferta.czas);

        if (serwis_ipc_wyslij_zlecenie(s, id_klienta, stanowisko_id, oferta) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad wysylki zlecenia" << std::endl;
            serwis_logf("pracownik", "blad wysylki zlecenia id_klienta=%d stanowisko=%d", id_klienta, stanowisko_id);
            break;
        }

        dl_kolejki--;
        aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
    }

    serwis_ipc_cleanup();
    serwis_log("pracownik", "koniec");
    std::cout << "[pracownik_serwisu] koniec" << std::endl;
    return EXIT_SUCCESS;
}
