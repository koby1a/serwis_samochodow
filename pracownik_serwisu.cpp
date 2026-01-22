#include <unistd.h>
#include "serwis_ipc.h"
#include "model.h"
#include "logger.h"

static const int TP = 8 * 60;
static const int TK = 16 * 60;
static const int T1 = 60;
static const int K1 = 3;
static const int K2 = 5;
<<<<<<< HEAD

/**
 * @brief Sprawdza dostepnosc stanowiska na podstawie SHM.
 */
=======
static const int KRIT_SERVICES[3] = {11, 29, 8};

/** @brief Sprawdza dostepnosc stanowiska na podstawie SHM. */
>>>>>>> 49aa6d4 (v20)
static int stanowisko_dostepne(const SerwisStatystyki& st, int id) {
    if (id < 1 || id > 8) return 0;
    if (st.st[id].zamkniete) return 0;
    if (st.req_close[id]) return 0;
    return 1;
}

<<<<<<< HEAD
/**
 * @brief Wybiera stanowisko zgodnie z ograniczeniami i zamknieciami.
 */
=======
/** @brief Wybiera stanowisko zgodnie z ograniczeniami i zamknieciami. */
>>>>>>> 49aa6d4 (v20)
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

<<<<<<< HEAD
=======
/** @brief Proces pracownika: rejestracja, oferta, wybor stanowiska, wysylka zlecen. */
>>>>>>> 49aa6d4 (v20)
int main() {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    unsigned int seed = 12345u;
    int okienka = 1;
    int kolejka = 0;
    int next_client = 1;

    serwis_log("pracownik", "start");

    while (!serwis_get_pozar()) {
<<<<<<< HEAD
        Samochod s{};
        if (serwis_ipc_recv_zgl(s) != SERWIS_IPC_OK) break;
=======
        while (true) {
            Raport r{};
            int rr = serwis_ipc_try_recv_rap(r);
            if (rr == SERWIS_IPC_NO_MSG) break;
            if (rr != SERWIS_IPC_OK) { if (serwis_get_pozar()) break; continue; }
            serwis_logf("pracownik", "odbior_formularza id=%d st=%d koszt=%d czas=%d",
                        r.id_klienta, r.stanowisko_id, r.koszt, r.czas);
            (void)serwis_ipc_send_kasa(r);
        }

        while (true) {
            SerwisExtraReq er{};
            int erc = serwis_ipc_try_recv_extra_req(er);
            if (erc == SERWIS_IPC_NO_MSG) break;
            if (erc != SERWIS_IPC_OK) { if (serwis_get_pozar()) break; continue; }
            int los = serwis_losuj_int(&seed, 0, 99);
            int akcept = serwis_klient_zgadza_sie_na_rozszerzenie(los, 20) ? 1 : 0;
            serwis_logf("pracownik", "dodatkowe id=%d st=%d czas=%d koszt=%d akcept=%d",
                        er.id_klienta, er.stanowisko_id, er.czas_dod, er.koszt_dod, akcept);
            SerwisExtraResp resp{};
            resp.id_klienta = er.id_klienta;
            resp.akceptacja = akcept;
            (void)serwis_ipc_send_extra_resp(resp);
        }

        Samochod s{};
        int zr = serwis_ipc_try_recv_zgl(s);
        if (zr == SERWIS_IPC_NO_MSG) {
            usleep(50000);
            continue;
        }
        if (zr != SERWIS_IPC_OK) {
            if (serwis_get_pozar()) break;
            continue;
        }
>>>>>>> 49aa6d4 (v20)

        kolejka++;
        okienka = serwis_aktualizuj_okienka(okienka, kolejka, K1, K2);

        if (!serwis_czy_moze_czekac_poza_godzinami(TP, TK, T1, s)) {
<<<<<<< HEAD
            serwis_logf("pracownik", "odrzut poza_godzinami marka=%c t=%d", s.marka, s.czas_przyjazdu);
=======
            serwis_logf("pracownik", "odrzut poza_godzinami marka=%c t=%d kryt=%d typ=%d",
                        s.marka, s.czas_przyjazdu, s.krytyczna, s.krytyczna_typ);
>>>>>>> 49aa6d4 (v20)
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
<<<<<<< HEAD
=======
        if (s.krytyczna && s.krytyczna_typ >= 1 && s.krytyczna_typ <= 3) {
            int krit_id = KRIT_SERVICES[s.krytyczna_typ - 1];
            oferta.uslugi[0] = krit_id;
            oferta.koszt = serwis_oblicz_koszt(oferta.uslugi, oferta.liczba_uslug);
            oferta.czas  = serwis_oblicz_czas_z_uslug(oferta.uslugi, oferta.liczba_uslug, 0, SERWIS_TRYB_NORMALNY);
        }
>>>>>>> 49aa6d4 (v20)

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

<<<<<<< HEAD
        serwis_logf("pracownik", "zlecenie id=%d st=%d marka=%c koszt=%d czas=%d okienka=%d",
                    z.id_klienta, stid, s.marka, oferta.koszt, oferta.czas, okienka);

        if (serwis_ipc_send_zlec(z) != SERWIS_IPC_OK) break;
=======
        serwis_logf("pracownik", "zlecenie id=%d st=%d marka=%c koszt=%d czas=%d okienka=%d kryt=%d typ=%d",
                    z.id_klienta, stid, s.marka, oferta.koszt, oferta.czas, okienka,
                    s.krytyczna, s.krytyczna_typ);

        if (serwis_ipc_send_zlec(z) != SERWIS_IPC_OK) {
            if (serwis_get_pozar()) break;
        }
>>>>>>> 49aa6d4 (v20)

        kolejka--;
    }

    serwis_log("pracownik", "koniec");
    serwis_ipc_detach();
    return 0;
}
