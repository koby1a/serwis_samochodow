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

int main(int argc, char** argv) {
    int n = serwis_parse_int(argc, argv, "--n", 50);
    int sleep_ms = serwis_parse_int(argc, argv, "--sleep_ms", 0);
    unsigned int seed = static_cast<unsigned int>(serwis_parse_int(argc, argv, "--seed", 2026));

    // Opcjonalnie: --log plik.log
    std::string log_path = "raport_symulacji.log";
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--log") {
            log_path = argv[i + 1];
        }
    }
    serwis_logger_set_file(log_path.c_str());

    std::cout << "[kierowca] start (n=" << n << ", seed=" << seed << ")" << std::endl;
    serwis_logf("kierowca", "start n=%d seed=%u", n, seed);

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[kierowca] blad ipc_init" << std::endl;
        serwis_log("kierowca", "blad ipc_init");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; ++i) {
        Samochod s{};
        s.marka = static_cast<char>('A' + serwis_losuj_int(&seed, 0, 25));
        s.czas_przyjazdu = serwis_losuj_int(&seed, 0, 1439);
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

#if defined(__unix__) || defined(__APPLE__)
        if (sleep_ms > 0) usleep(static_cast<useconds_t>(sleep_ms) * 1000u);
#else
        (void)sleep_ms;
#endif
    }

    serwis_ipc_cleanup();
    serwis_log("kierowca", "koniec");
    std::cout << "[kierowca] koniec" << std::endl;
    return EXIT_SUCCESS;
}
