#include <cstdlib>
#include <string>
#include <csignal>
#include <unistd.h>
#include "serwis_ipc.h"
#include "model.h"
#include "logger.h"

static SerwisTrybPracy g_tryb = SERWIS_TRYB_NORMALNY;
static int g_zamknij_po = 0;

/** @brief Handler sygnalow mechanika. */
static void on_sig(int sig) {
    if (sig == SIGUSR1) g_zamknij_po = 1;
    else if (sig == SIGUSR2) { if (g_tryb == SERWIS_TRYB_NORMALNY) g_tryb = SERWIS_TRYB_PRZYSPIESZONY; }
    else if (sig == SIGTERM) { if (g_tryb == SERWIS_TRYB_PRZYSPIESZONY) g_tryb = SERWIS_TRYB_NORMALNY; }
    else if (sig == SIGINT) serwis_set_pozar(1);
}

/** @brief Rejestruje sygnaly. */
static void regsig() {
    struct sigaction sa{};
    sa.sa_handler = on_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGUSR1");
    if (sigaction(SIGUSR2, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGUSR2");
    if (sigaction(SIGTERM, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGTERM");
    if (sigaction(SIGINT, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGINT");
}

/** @brief Pobiera int z argv. */
static int argi(int argc, char** argv, const char* k, int d) {
    for (int i = 1; i + 1 < argc; ++i) if (std::string(argv[i]) == k) return std::atoi(argv[i + 1]);
    return d;
}

int main(int argc, char** argv) {
    serwis_logger_set_file("raport_symulacji.log");
    int id = argi(argc, argv, "--id", 1);
    if (id < 1 || id > 8) return 1;

    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;
    regsig();

    serwis_station_set_pid(id, (int)getpid());
    serwis_logf("mechanik", "start id=%d pid=%d", id, (int)getpid());

    while (!serwis_get_pozar()) {
        if (serwis_get_req_close(id)) g_zamknij_po = 1;

        Zlecenie z{};
        if (serwis_ipc_recv_zlec(id, z) != SERWIS_IPC_OK) {
            if (serwis_get_pozar()) break;
            continue;
        }
        if (serwis_get_pozar()) break;

        serwis_station_set_busy(id, 1, z.s.marka, z.s.krytyczna, 0, (int)g_tryb);

        unsigned int seed = (unsigned int)(id * 1111 + z.id_klienta * 7);
        int dodatkowe = 0;
        int czas_dod = 0;
        int koszt_dod = 0;

        if (serwis_losuj_int(&seed, 0, 99) < 20) {
            int proponowany_czas = serwis_losuj_int(&seed, 10, 60);
            int proponowany_koszt = serwis_losuj_int(&seed, 100, 500);

            SerwisExtraReq req{};
            req.id_klienta = z.id_klienta;
            req.stanowisko_id = id;
            req.czas_dod = proponowany_czas;
            req.koszt_dod = proponowany_koszt;
            (void)serwis_ipc_send_extra_req(req);

            SerwisExtraResp resp{};
            if (serwis_ipc_recv_extra_resp(z.id_klienta, resp) == SERWIS_IPC_OK && resp.akceptacja) {
                dodatkowe = 1;
                czas_dod = proponowany_czas;
                koszt_dod = proponowany_koszt;
                serwis_logf("mechanik", "dodatkowe zaakceptowane id=%d st=%d", z.id_klienta, id);
            } else {
                serwis_logf("mechanik", "dodatkowe odrzucone id=%d st=%d", z.id_klienta, id);
            }
        }

        serwis_station_set_busy(id, 1, z.s.marka, z.s.krytyczna, dodatkowe, (int)g_tryb);

        int czas = serwis_oblicz_czas_naprawy(z.oferta.czas, czas_dod, g_tryb);
        int koszt = z.oferta.koszt + koszt_dod;

        usleep((useconds_t)(czas * 1000));

        Raport r{};
        r.id_klienta = z.id_klienta;
        r.stanowisko_id = id;
        r.s = z.s;
        r.czas = czas;
        r.koszt = koszt;

        (void)serwis_ipc_send_rap(r);

        serwis_station_inc_done(id);
        serwis_station_set_busy(id, 0, '-', 0, 0, (int)g_tryb);

        if (g_zamknij_po) {
            serwis_station_set_closed(id, 1);
            serwis_req_close(id, 0);
            break;
        }
    }

    serwis_station_set_pid(id, 0);
    serwis_logf("mechanik", "koniec id=%d", id);
    serwis_ipc_detach();
    return 0;
}
