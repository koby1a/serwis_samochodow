#include <string>
#include <vector>
#include <cstdlib>
#include "serwis_ipc.h"
#include "logger.h"

static int argi(int argc, char** argv, const char* k, int d) {
    for (int i = 1; i + 1 < argc; ++i) if (std::string(argv[i]) == k) return std::atoi(argv[i + 1]);
    return d;
}

static std::string args(int argc, char** argv, const char* k, const std::string& d) {
    for (int i = 1; i + 1 < argc; ++i) if (std::string(argv[i]) == k) return argv[i + 1];
    return d;
}

/** @brief Proces kasjera: odbiera platnosci od pracownika. */
int main(int argc, char** argv) {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;
    int time_scale = argi(argc, argv, "--time_scale", 10);
    std::string scenario = args(argc, argv, "--scenario", "");
    serwis_time_scale_set(time_scale);

    serwis_log("kasjer", "start");

    std::vector<Raport> buffered;
    bool defer_logs = (scenario == "T3");

    while (!serwis_get_pozar()) {
        Raport r{};
        int rr = serwis_ipc_recv_kasa(r);
        if (rr == SERWIS_IPC_SHUTDOWN) break;
        if (rr != SERWIS_IPC_OK) {
            if (serwis_get_pozar()) break;
            continue;
        }
        if (defer_logs) buffered.push_back(r);
        else serwis_logf("kasjer", "platnosc id=%d st=%d marka=%c koszt=%d czas=%d",
                         r.id_klienta, r.stanowisko_id, r.s.marka, r.koszt, r.czas);
    }

    if (defer_logs) {
        for (const auto& r : buffered) {
            serwis_logf("kasjer", "platnosc id=%d st=%d marka=%c koszt=%d czas=%d",
                        r.id_klienta, r.stanowisko_id, r.s.marka, r.koszt, r.czas);
        }
    }

    serwis_log("kasjer", "koniec");
    serwis_ipc_detach();
    return 0;
}
