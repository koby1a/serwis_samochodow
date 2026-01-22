#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include "serwis_ipc.h"
#include "model.h"
#include "logger.h"

/**
 * @brief Pobiera int z argv.
 */
static int argi(int argc, char** argv, const std::string& k, int d) {
    for (int i = 1; i + 1 < argc; ++i) if (k == argv[i]) return std::atoi(argv[i + 1]);
    return d;
}

static std::string args(int argc, char** argv, const std::string& k, const std::string& d) {
    for (int i = 1; i + 1 < argc; ++i) if (k == argv[i]) return argv[i + 1];
    return d;
}

int main(int argc, char** argv) {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    int n = argi(argc, argv, "--n", 200);
    int sleep_ms = argi(argc, argv, "--sleep_ms", 5);
    int time_offset_range = argi(argc, argv, "--time_offset_range", 180);
    unsigned int seed = (unsigned int)argi(argc, argv, "--seed", 2026);
    std::string scenario = args(argc, argv, "--scenario", "default");

    if (n < 1) n = 1;
    if (n > 50000) n = 50000;
    if (sleep_ms < 0) sleep_ms = 0;
    if (sleep_ms > 5000) sleep_ms = 5000;
    if (time_offset_range < 0) time_offset_range = 0;
    if (time_offset_range > 720) time_offset_range = 720;

    serwis_logf("kierowca", "start n=%d seed=%u scen=%s", n, seed, scenario.c_str());

    std::vector<Samochod> fixed;
    if (scenario == "T1") {
        const char brands[] = {'A','E','I','O','U','U','U','Y','Y','Y','B','C','Z'};
        for (char b : brands) {
            Samochod s{};
            s.marka = b;
            s.czas_przyjazdu = serwis_time_get();
            s.krytyczna = 0;
            s.krytyczna_typ = 0;
            s.czas_naprawy = 0;
            fixed.push_back(s);
        }
    }

    int sent = 0;
    for (size_t i = 0; i < fixed.size() && sent < n && !serwis_get_pozar(); ++i, ++sent) {
        if (serwis_ipc_send_zgl(fixed[i]) != SERWIS_IPC_OK) break;
        usleep((useconds_t)sleep_ms * 1000u);
    }

    for (; sent < n && !serwis_get_pozar(); ++sent) {
        Samochod s{};
        s.marka = (char)('A' + serwis_losuj_int(&seed, 0, 25));
        int sim_t = serwis_time_get();
        int offset = serwis_losuj_int(&seed, -time_offset_range, time_offset_range);
        int t = sim_t + offset;
        if (t < 0) t += 1440;
        if (t >= 1440) t %= 1440;
        s.czas_przyjazdu = t;
        s.krytyczna = (serwis_losuj_int(&seed, 0, 99) < 10) ? 1 : 0;
        s.krytyczna_typ = s.krytyczna ? serwis_losuj_int(&seed, 1, 3) : 0;
        s.czas_naprawy = 0;

        if (serwis_ipc_send_zgl(s) != SERWIS_IPC_OK) break;
        usleep((useconds_t)sleep_ms * 1000u);
    }

    serwis_log("kierowca", "koniec");
    serwis_ipc_detach();
    return 0;
}
