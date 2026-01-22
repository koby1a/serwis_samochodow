#include <cstdlib>
#include <string>
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

int main(int argc, char** argv) {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    int n = argi(argc, argv, "--n", 200);
    int sleep_ms = argi(argc, argv, "--sleep_ms", 5);
    unsigned int seed = (unsigned int)argi(argc, argv, "--seed", 2026);

    if (n < 1) n = 1;
    if (n > 50000) n = 50000;
    if (sleep_ms < 0) sleep_ms = 0;
    if (sleep_ms > 5000) sleep_ms = 5000;

    serwis_logf("kierowca", "start n=%d seed=%u", n, seed);

    for (int i = 0; i < n && !serwis_get_pozar(); ++i) {
        Samochod s{};
        s.marka = (char)('A' + serwis_losuj_int(&seed, 0, 25));
        s.czas_przyjazdu = serwis_losuj_int(&seed, 0, 1439);
        s.krytyczna = (serwis_losuj_int(&seed, 0, 99) < 10) ? 1 : 0;
        s.czas_naprawy = 0;

        if (serwis_ipc_send_zgl(s) != SERWIS_IPC_OK) break;
        usleep((useconds_t)sleep_ms * 1000u);
    }

    serwis_log("kierowca", "koniec");
    serwis_ipc_detach();
    return 0;
}
