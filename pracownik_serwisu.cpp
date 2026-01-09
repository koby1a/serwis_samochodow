// pracownik_serwisu.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"
#include "stanowiska_status.h"

// Godziny serwisu (minuty w dobie): 08:00-16:00
static const int TP = 8 * 60;
static const int TK = 16 * 60;

// Jezeli do otwarcia mniej niz T1, klient moze poczekac
static const int T1 = 60;

// K1/K2
static const int K1 = 3;
static const int K2 = 5;

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
    // Dostepne tylko gdy nie ma prosby o zamkniecie i nie jest zamkniete
    if (serwis_stanowisko_ma_prosbe_zamkniecia(id)) return 0;
    if (serwis_stanowisko_jest_zamkniete(id)) return 0;
    return 1;
}

static int serwis_wybierz_stanowisko_dla_marki(char marka, unsigned int* seed) {
    if (marka >= 'a' && marka <= 'z') marka = static_cast<char>(marka - 'a' + 'A');

    // U/Y moze trafic na 8, ale tylko jesli dostepne
    if (marka == 'U' || marka == 'Y') {
        int r = serwis_losuj_int(seed, 0, 99);
        if (r < 40 && serwis_stanowisko_dostepne(8)) {
            return 8;
        }
        // inaczej szukamy 1..7
    }

    // Round-robin po 1..7, ale pomijamy niedostepne
    static int rr = 1;
    for (int probe = 0; probe < 7; ++probe) {
        int cand = rr;
        rr++;
        if (rr > 7) rr = 1;

        if (serwis_stanowisko_dostepne(cand)) {
            return cand;
        }
    }

    // Jesli nic z 1..7, a U/Y to moze jeszcze 8 (jesli akurat otwarte)
    if ((marka == 'U' || marka == 'Y') && serwis_stanowisko_dostepne(8)) {
        return 8;
    }

    return -1;
}

int main() {
    std::cout << "[pracownik_serwisu] start" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[pracownik_serwisu] blad ipc_init" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[pracownik_serwisu] parametry: Tp=" << TP
              << ", Tk=" << TK
              << ", T1=" << T1
              << ", K1=" << K1
              << ", K2=" << K2
              << std::endl;

    unsigned int seed = 12345u;
    int nastepne_id_klienta = 1;

    int aktywne_okienka = 1;
    int dl_kolejki = 0;

    const int MAKS_ZGLOSZEN = 500;

    for (int iter = 0; iter < MAKS_ZGLOSZEN; ++iter) {
        if (serwis_pozar_jest()) {
            std::cout << "[pracownik_serwisu] pozar.flag -> koniec" << std::endl;
            break;
        }

        Samochod s{};
        std::cout << "[pracownik_serwisu] czekam na zgloszenie" << std::endl;

        if (serwis_ipc_odbierz_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad odbioru zgloszenia" << std::endl;
            break;
        }

        std::cout << "[pracownik_serwisu] zgloszenie: marka=" << s.marka
                  << ", czas_przyjazdu=" << s.czas_przyjazdu
                  << ", krytyczna=" << s.krytyczna
                  << std::endl;

        // Godziny otwarcia
        if (!serwis_czy_w_godzinach(s.czas_przyjazdu)) {
            int do_otwarcia = serwis_minuty_do_otwarcia(s.czas_przyjazdu);

            if (!serwis_czy_moze_czekac_poza_godzinami(s)) {
                std::cout << "[pracownik_serwisu] poza godzinami, klient nie moze czekac (do_otwarcia="
                          << do_otwarcia << ") -> odjezdza" << std::endl;
                continue;
            }

            std::cout << "[pracownik_serwisu] poza godzinami, klient czeka (do_otwarcia="
                      << do_otwarcia << ", krytyczna=" << s.krytyczna << ")" << std::endl;
        }

        // Kolejka rejestracji + okienka
        dl_kolejki++;
        int stare = aktywne_okienka;
        aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
        if (stare != aktywne_okienka) {
            std::cout << "[pracownik_serwisu] zmiana okienek: " << stare
                      << " -> " << aktywne_okienka << " (kolejka=" << dl_kolejki << ")" << std::endl;
        }

        if (!serwis_czy_marka_obslugiwana(s.marka)) {
            std::cout << "[pracownik_serwisu] marka nieobslugiwana -> klient odjezdza" << std::endl;
            dl_kolejki--;
            aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
            continue;
        }

        OfertaNaprawy oferta{};
        if (!serwis_utworz_oferte(&oferta, &seed, 2, 5, 0, SERWIS_TRYB_NORMALNY)) {
            std::cout << "[pracownik_serwisu] nie udalo sie utworzyc oferty" << std::endl;
            dl_kolejki--;
            aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
            continue;
        }

        int los = serwis_losuj_int(&seed, 0, 99);
        if (!serwis_klient_akceptuje_warunki(los, 2)) {
            std::cout << "[pracownik_serwisu] klient odrzucil oferte -> odjezdza" << std::endl;
            dl_kolejki--;
            aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
            continue;
        }

        int stanowisko_id = serwis_wybierz_stanowisko_dla_marki(s.marka, &seed);
        if (stanowisko_id == -1) {
            std::cout << "[pracownik_serwisu] brak dostepnych stanowisk -> klient odjezdza" << std::endl;
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

        if (serwis_ipc_wyslij_zlecenie(s, id_klienta, stanowisko_id, oferta) != SERWIS_IPC_OK) {
            std::cerr << "[pracownik_serwisu] blad wysylki zlecenia" << std::endl;
            break;
        }

        dl_kolejki--;
        aktywne_okienka = serwis_aktualizuj_okienka(aktywne_okienka, dl_kolejki);
    }

    serwis_ipc_cleanup();
    std::cout << "[pracownik_serwisu] koniec" << std::endl;
    return EXIT_SUCCESS;
}
