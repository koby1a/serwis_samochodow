#include <string>
#include <vector>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include "serwis_ipc.h"
#include "model.h"
#include "logger.h"
#include "time_scale.h"

static int TP = 8 * 60;
static int TK = 16 * 60;
static int T1 = 60;
static int K1 = 3;
static int K2 = 5;
static const int KRIT_SERVICES[3] = {11, 29, 8};

static volatile sig_atomic_t g_active = 1;
static volatile sig_atomic_t g_stop = 0;

/** @brief Pobiera int z argv. */
static int argi(int argc, char** argv, const char* k, int d) {
    for (int i = 1; i + 1 < argc; ++i) if (std::string(argv[i]) == k) return std::atoi(argv[i + 1]);
    return d;
}

/** @brief Sprawdza dostepnosc stanowiska na podstawie SHM. */
static int stanowisko_dostepne(const SerwisStatystyki& st, int id) {
    if (id < 1 || id > 8) return 0;
    if (st.st[id].zamkniete) return 0;
    if (st.req_close[id]) return 0;
    return 1;
}

/** @brief Wybiera stanowisko zgodnie z ograniczeniami i zamknieciami. */
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

static void obsluz_sig_okienka(int sig) {
    if (sig == SIGUSR1) g_active = 0;
    else if (sig == SIGUSR2) g_active = 1;
    else if (sig == SIGINT || sig == SIGTERM) g_stop = 1;
}

static void zarejestruj_sig_okienka() {
    struct sigaction sa{};
    sa.sa_handler = obsluz_sig_okienka;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, nullptr) == -1) perror("[pracownik] sigaction SIGUSR1");
    if (sigaction(SIGUSR2, &sa, nullptr) == -1) perror("[pracownik] sigaction SIGUSR2");
    if (sigaction(SIGINT, &sa, nullptr) == -1) perror("[pracownik] sigaction SIGINT");
    if (sigaction(SIGTERM, &sa, nullptr) == -1) perror("[pracownik] sigaction SIGTERM");
}

static int wszystkie_stanowiska_wolne(const SerwisStatystyki& st) {
    for (int i = 1; i <= 8; ++i) if (st.st[i].zajete) return 0;
    return 1;
}

static int system_quiet() {
    SerwisQueueCounts qc{};
    SerwisStatystyki st{};
    if (serwis_ipc_get_queue_counts(qc) != SERWIS_IPC_OK) return 0;
    if (serwis_stat_get(st) != SERWIS_IPC_OK) return 0;
    if (!wszystkie_stanowiska_wolne(st)) return 0;
    if (qc.zgl || qc.zlec || qc.rap || qc.kasa || qc.ext_req || qc.ext_resp) return 0;
    return 1;
}

static int utworz_id_klienta(int& local) {
    int pid = (int)getpid() & 0x7FFF;
    int id = (pid << 16) | (local & 0xFFFF);
    local++;
    return id;
}

static void petla_okienka(int worker_id, int leader, int time_scale) {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) _exit(1);
    serwis_time_scale_set(time_scale);
    zarejestruj_sig_okienka();

    unsigned int seed = (unsigned int)(12345u + (unsigned int)worker_id * 777u);
    int okienka = 1;
    int next_client = 1;
    int zgl_done = 0;

    serwis_logf("pracownik", "start worker=%d leader=%d", worker_id, leader);

    while (!serwis_get_pozar() && !g_stop) {
        while (true) {
            Raport r{};
            int rr = serwis_ipc_try_recv_rap(r);
            if (rr == SERWIS_IPC_NO_MSG) break;
            if (rr == SERWIS_IPC_SHUTDOWN) break;
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
            int akcept = 0;
            if (serwis_klient_zgadza_sie_na_rozszerzenie(los, 20)) akcept = 1;
            serwis_logf("pracownik", "dodatkowe id=%d st=%d czas=%d koszt=%d akcept=%d",
                        er.id_klienta, er.stanowisko_id, er.czas_dod, er.koszt_dod, akcept);
            SerwisExtraResp resp{};
            resp.id_klienta = er.id_klienta;
            resp.akceptacja = akcept;
            (void)serwis_ipc_send_extra_resp(resp);
        }

        if (!g_active) {
            serwis_sleep_us_scaled(50000, time_scale);
            continue;
        }

        Samochod s{};
        int zr = serwis_ipc_try_recv_zgl(s);
        if (zr == SERWIS_IPC_SHUTDOWN) {
            if (leader) zgl_done = 1;
            else break;
        } else if (zr == SERWIS_IPC_NO_MSG) {
            if (leader && zgl_done) {
                if (system_quiet()) {
                    for (int id = 1; id <= 8; ++id) (void)serwis_ipc_send_zlec_shutdown(id);
                    (void)serwis_ipc_send_kasa_shutdown();
                    break;
                }
            }
            serwis_sleep_us_scaled(50000, time_scale);
            continue;
        }
        if (zr != SERWIS_IPC_OK) {
            if (serwis_get_pozar()) break;
            continue;
        }

        SerwisQueueCounts qc{};
        if (serwis_ipc_get_queue_counts(qc) == SERWIS_IPC_OK) {
            okienka = serwis_aktualizuj_okienka(okienka, (int)qc.zgl, K1, K2);
        }

        if (!serwis_czy_moze_czekac_poza_godzinami(TP, TK, T1, s)) {
            serwis_logf("pracownik", "odrzut poza_godzinami marka=%c t=%d kryt=%d typ=%d",
                        s.marka, s.czas_przyjazdu, s.krytyczna, s.krytyczna_typ);
            continue;
        }

        if (!serwis_czy_marka_obslugiwana(s.marka)) {
            serwis_logf("pracownik", "odrzut nieobslugiwana marka=%c", s.marka);
            continue;
        }

        OfertaNaprawy oferta{};
        if (!serwis_utworz_oferte(&oferta, &seed, 2, 5, 0, SERWIS_TRYB_NORMALNY)) {
            continue;
        }
        if (s.krytyczna && s.krytyczna_typ >= 1 && s.krytyczna_typ <= 3) {
            int krit_id = KRIT_SERVICES[s.krytyczna_typ - 1];
            oferta.uslugi[0] = krit_id;
            oferta.koszt = serwis_oblicz_koszt(oferta.uslugi, oferta.liczba_uslug);
            oferta.czas  = serwis_oblicz_czas_z_uslug(oferta.uslugi, oferta.liczba_uslug, 0, SERWIS_TRYB_NORMALNY);
        }

        int los = serwis_losuj_int(&seed, 0, 99);
        if (!serwis_klient_akceptuje_warunki(los, 2)) {
            serwis_logf("pracownik", "odrzut oferty los=%d", los);
            continue;
        }

        int stid = wybierz_stanowisko(s.marka, &seed);
        if (stid == -1) {
            serwis_log("pracownik", "brak stanowiska -> odrzut");
            continue;
        }

        Zlecenie z{};
        z.id_klienta = utworz_id_klienta(next_client);
        z.stanowisko_id = stid;
        z.s = s;
        z.oferta = oferta;

        serwis_logf("pracownik", "zlecenie id=%d st=%d marka=%c koszt=%d czas=%d okienka=%d kryt=%d typ=%d",
                    z.id_klienta, stid, s.marka, oferta.koszt, oferta.czas, okienka,
                    s.krytyczna, s.krytyczna_typ);

        if (serwis_ipc_send_zlec(z) != SERWIS_IPC_OK) {
            if (serwis_get_pozar()) break;
        }
    }

    serwis_logf("pracownik", "koniec worker=%d", worker_id);
    serwis_ipc_detach();
    _exit(0);
}

static void ustaw_okienko_aktywne(pid_t pid, int active) {
    if (pid <= 0) return;
    int sig = SIGUSR1;
    if (active) sig = SIGUSR2;
    (void)kill(pid, sig);
}

/** @brief Proces pracownika: rejestracja, oferta, wybor stanowiska, obsluga raportow. */
int main(int argc, char** argv) {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    TP = argi(argc, argv, "--tp", TP);
    TK = argi(argc, argv, "--tk", TK);
    T1 = argi(argc, argv, "--t1", T1);
    K1 = argi(argc, argv, "--k1", K1);
    K2 = argi(argc, argv, "--k2", K2);
    int time_scale = argi(argc, argv, "--time_scale", 10);
    int workers = argi(argc, argv, "--workers", 3);
    if (workers < 1) workers = 1;
    if (workers > 3) workers = 3;
    serwis_time_scale_set(time_scale);

    std::vector<pid_t> pids;
    pids.reserve((size_t)workers);
    for (int i = 0; i < workers; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            petla_okienka(i + 1, i == 0, time_scale);
        }
        if (pid > 0) pids.push_back(pid);
    }

    int active = 1;
    for (int i = 1; i < workers; ++i) ustaw_okienko_aktywne(pids[i], 0);

    serwis_log("pracownik", "start parent");

    while (!serwis_get_pozar()) {
        SerwisQueueCounts qc{};
        if (serwis_ipc_get_queue_counts(qc) == SERWIS_IPC_OK) {
            int new_active = serwis_aktualizuj_okienka(active, (int)qc.zgl, K1, K2);
            if (new_active != active) {
                if (new_active > active) {
                    for (int i = active; i < new_active; ++i) {
                        ustaw_okienko_aktywne(pids[i], 1);
                        serwis_logf("pracownik", "okienko_open id=%d", i + 1);
                    }
                } else {
                    for (int i = active - 1; i >= new_active; --i) {
                        ustaw_okienko_aktywne(pids[i], 0);
                        serwis_logf("pracownik", "okienko_close id=%d", i + 1);
                    }
                }
                active = new_active;
            }
        }

        int status = 0;
        pid_t p;
        while ((p = waitpid(-1, &status, WNOHANG)) > 0) {
            for (size_t i = 0; i < pids.size(); ++i) {
                if (pids[i] == p) pids[i] = -1;
            }
        }

        bool any_alive = false;
        for (pid_t pid : pids) if (pid > 0) any_alive = true;
        if (!any_alive) break;

        serwis_sleep_us_scaled(50000, time_scale);
    }

    for (pid_t pid : pids) if (pid > 0) kill(pid, SIGINT);
    for (pid_t pid : pids) if (pid > 0) waitpid(pid, nullptr, 0);

    serwis_log("pracownik", "koniec parent");
    serwis_ipc_detach();
    return 0;
}
