#include <unistd.h>
#include "serwis_ipc.h"
#include "model.h"
#include "logger.h"

static const int TP = 8 * 60;
static const int TK = 16 * 60;
static const int T1 = 60;
static const int K1 = 3;
static const int K2 = 5;

/**
 * @brief Sprawdza dostepnosc stanowiska na podstawie SHM.
 */
static int stanowisko_dostepne(const SerwisStatystyki& st, int id) {
    if (id < 1 || id > 8) return 0;
    if (st.st[id].zamkniete) return 0;
    if (st.req_close[id]) return 0;
    return 1;
}

/**
 * @brief Wybiera stanowisko zgodnie z ograniczeniami i zamknieciami.
 */
static int wybierz_stanowisko(char marka, unsigned int* seed) {
    SerwisStatystyki st{};
    if (serwis_stat_get(st) != SERWIS_IPC_OK) return -1;

    if ((marka == 'U' || marka == 'Y') && serwis_losuj_int(seed, 0, 99) < 40) {
        if (stanowisko_dostepne(st, 8)) return 8;
    }

    static int rr = 1;
    for (int k = 0; k < 7; ++k) {
        int cand = rr;
        rr++; if (rr > 7) rr = 1;
        if (stanowisko_dostepne(st, cand) && serwis_stanowisko_moze_obsluzyc(cand, marka)) return cand;
    }

    if ((marka == 'U' || marka == 'Y') && stanowisko_dostepne(st, 8)) return 8;
    return -1;
}

int main() {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    unsigned int seed = 12345u;
    int okienka = 1;
    int kolejka = 0;
    int next_client = 1;

    serwis_log("pracownik", "start");

    while (!serwis_get_pozar()) {
        Samochod s{};
        if (serwis_ipc_recv_zgl(s) != SERWIS_IPC_OK) break;

        kolejka++;
        okienka = serwis_aktualizuj_okienka(okienka, kolejka, K1, K2);

        if (!serwis_czy_moze_czekac_poza_godzinami(TP, TK, T1, s)) {
            serwis_logf("pracownik", "odrzut poza_godzinami marka=%c t=%d", s.marka, s.czas_przyjazdu);
            kolejka--;
            continue;
        }

        if (!serwis_czy_marka_obslugiwana(s.marka)) {
            serwis_logf("pracownik", "odrzut nieobslugiwana marka=%c", s.marka);
            kolejka--;
            continue;
        }

        OfertaNaprawy oferta{};
        if (!serwis_utworz_oferte(&oferta, &seed, 2, 5, 0, SERWIS_TRYB_NORMALNY)) {
            kolejka--;
            continue;
        }

        int los = serwis_losuj_int(&seed, 0, 99);
        if (!serwis_klient_akceptuje_warunki(los, 2)) {
            serwis_logf("pracownik", "odrzut oferty los=%d", los);
            kolejka--;
            continue;
        }

        int stid = wybierz_stanowisko(s.marka, &seed);
        if (stid == -1) {
            serwis_log("pracownik", "brak stanowiska -> odrzut");
            kolejka--;
            continue;
        }

        Zlecenie z{};
        z.id_klienta = next_client++;
        z.stanowisko_id = stid;
        z.s = s;
        z.oferta = oferta;

        serwis_logf("pracownik", "zlecenie id=%d st=%d marka=%c koszt=%d czas=%d okienka=%d",
                    z.id_klienta, stid, s.marka, oferta.koszt, oferta.czas, okienka);

        if (serwis_ipc_send_zlec(z) != SERWIS_IPC_OK) break;

        kolejka--;
    }

    serwis_log("pracownik", "koniec");
    serwis_ipc_detach();
    return 0;
}
