// kierowca.cpp
#include <iostream>
#include <cstdlib>
#include <string>
#include "model.h"
#include "serwis_ipc.h"

#if defined(__unix__) || defined(__APPLE__)
    #include <unistd.h> // usleep
#endif

static int serwis_parse_int(int argc, char** argv, const std::string& key, int domyslna) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == key) {
            return std::atoi(argv[i + 1]);
        }
    }
    return domyslna;
}

int main(int argc, char** argv) {
    int n = serwis_parse_int(argc, argv, "--n", 50);
    int sleep_ms = serwis_parse_int(argc, argv, "--sleep_ms", 0);
    unsigned int seed = static_cast<unsigned int>(serwis_parse_int(argc, argv, "--seed", 2026));

    if (n < 1) n = 1;
    if (n > 10000) n = 10000;
    if (sleep_ms < 0) sleep_ms = 0;
    if (sleep_ms > 5000) sleep_ms = 5000;

    std::cout << "[kierowca] start (n=" << n
              << ", seed=" << seed
              << ", sleep_ms=" << sleep_ms
              << ")" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[kierowca] blad ipc_init" << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; ++i) {
        Samochod s{};

        // Marka A..Z
        int r = serwis_losuj_int(&seed, 0, 25);
        s.marka = static_cast<char>('A' + r);

        // Czas przyjazdu: 0..1439 (minuty w dobie)
        s.czas_przyjazdu = serwis_losuj_int(&seed, 0, 1439);

        // Krytyczna usterka ~10%
        int rk = serwis_losuj_int(&seed, 0, 99);
        s.krytyczna = (rk < 10) ? 1 : 0;

        // Czas naprawy ustalany pozniej (w ofercie / u mechanika)
        s.czas_naprawy = 0;

        std::cout << "[kierowca] zgloszenie #" << (i + 1)
                  << ": marka=" << s.marka
                  << ", czas_przyjazdu=" << s.czas_przyjazdu
                  << ", krytyczna=" << s.krytyczna
                  << std::endl;

        if (serwis_ipc_wyslij_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[kierowca] blad wysylki zgloszenia" << std::endl;
            break;
        }

#if defined(__unix__) || defined(__APPLE__)
        if (sleep_ms > 0) {
            usleep(static_cast<useconds_t>(sleep_ms) * 1000u);
        }
#else
        (void)sleep_ms;
#endif
    }

    serwis_ipc_cleanup();
    std::cout << "[kierowca] koniec" << std::endl;
    return EXIT_SUCCESS;
}
