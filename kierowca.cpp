// kierowca.cpp
#include <iostream>
#include <cstdlib>
#include <string>
#include "model.h"
#include "serwis_ipc.h"
#include "logger.h"

#if defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>
#endif

static int serwis_parse_int(int argc, char** argv, const std::string& key, int domyslna) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == key) {
            return std::atoi(argv[i + 1]);
        }
    }
    return domyslna;
}

static std::string serwis_parse_str(int argc, char** argv, const std::string& key, const std::string& domyslna) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == key) {
            return std::string(argv[i + 1]);
        }
    }
    return domyslna;
}

int main(int argc, char** argv) {
    int n = serwis_parse_int(argc, argv, "--n", 50);
    int sleep_ms = serwis_parse_int(argc, argv, "--sleep_ms", 0);
    unsigned int seed = static_cast<unsigned int>(serwis_parse_int(argc, argv, "--seed", 2026));
    std::string log_path = serwis_parse_str(argc, argv, "--log", "raport_symulacji.log");

    if (n < 1) n = 1;
    if (n > 20000) n = 20000;
    if (sleep_ms < 0) sleep_ms = 0;
    if (sleep_ms > 5000) sleep_ms = 5000;

    serwis_logger_set_file(log_path.c_str());

    std::cout << "[kierowca] start (n=" << n << ", seed=" << seed << ", sleep_ms=" << sleep_ms << ")" << std::endl;
    serwis_logf("kierowca", "start n=%d seed=%u sleep_ms=%d", n, seed, sleep_ms);

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[kierowca] blad ipc_init" << std::endl;
        serwis_log("kierowca", "blad ipc_init");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; ++i) {
        Samochod s{};

        // Marka A..Z
        s.marka = static_cast<char>('A' + serwis_losuj_int(&seed, 0, 25));

        // Losowy czas przyjazdu 0..1439 (minuty w dobie)
        s.czas_przyjazdu = serwis_losuj_int(&seed, 0, 1439);

        // Krytyczna usterka ~10%
        s.krytyczna = (serwis_losuj_int(&seed, 0, 99) < 10) ? 1 : 0;

        s.czas_naprawy = 0;

        std::cout << "[kierowca] zgloszenie #" << (i + 1)
                  << ": marka=" << s.marka
                  << ", czas_przyjazdu=" << s.czas_przyjazdu
                  << ", krytyczna=" << s.krytyczna
                  << std::endl;

        serwis_logf("kierowca", "zgloszenie marka=%c czas_przyjazdu=%d krytyczna=%d",
                    s.marka, s.czas_przyjazdu, s.krytyczna);

        if (serwis_ipc_wyslij_zgloszenie(s) != SERWIS_IPC_OK) {
            std::cerr << "[kierowca] blad wysylki zgloszenia" << std::endl;
            serwis_log("kierowca", "blad wysylki zgloszenia");
            break;
        }

        // Statystyki: przyjete zgloszenie (generator wyslal)
        serwis_stat_inc_przyjete();

#if defined(__unix__) || defined(__APPLE__)
        if (sleep_ms > 0) {
            usleep(static_cast<useconds_t>(sleep_ms) * 1000u);
        }
#else
        (void)sleep_ms;
#endif
    }

    serwis_ipc_cleanup();
    serwis_log("kierowca", "koniec");
    std::cout << "[kierowca] koniec" << std::endl;
    return EXIT_SUCCESS;
}
