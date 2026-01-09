// kasjer.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"
#include "logger.h"

int main(int argc, char** argv) {
    std::string log_path = "raport_symulacji.log";
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--log") {
            log_path = argv[i + 1];
        }
    }
    serwis_logger_set_file(log_path.c_str());

    std::cout << "[kasjer] start" << std::endl;
    serwis_log("kasjer", "start");

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[kasjer] blad ipc_init" << std::endl;
        serwis_log("kasjer", "blad ipc_init");
        return EXIT_FAILURE;
    }

    const int MAKS_RAPORTOW = 500;

    for (int i = 0; i < MAKS_RAPORTOW; ++i) {
        int id_klienta = 0;
        int rzeczywisty_czas = 0;
        int koszt_koncowy = 0;
        int stanowisko_id = 0;
        Samochod s{};

        std::cout << "[kasjer] czekalm na raport" << std::endl;

        if (serwis_ipc_odbierz_raport(id_klienta, rzeczywisty_czas, koszt_koncowy, stanowisko_id, s) != SERWIS_IPC_OK) {
            std::cerr << "[kasjer] blad odbioru raportu" << std::endl;
            serwis_log("kasjer", "blad odbioru raportu");
            break;
        }

        std::cout << "[kasjer] raport: id_klienta=" << id_klienta
                  << ", stanowisko=" << stanowisko_id
                  << ", marka=" << s.marka
                  << ", czas=" << rzeczywisty_czas
                  << ", koszt=" << koszt_koncowy
                  << std::endl;

        serwis_logf("kasjer", "platnosc id_klienta=%d stanowisko=%d marka=%c czas=%d koszt=%d",
                    id_klienta, stanowisko_id, s.marka, rzeczywisty_czas, koszt_koncowy);
    }

    serwis_ipc_cleanup();
    serwis_log("kasjer", "koniec");
    std::cout << "[kasjer] koniec" << std::endl;
    return EXIT_SUCCESS;
}
