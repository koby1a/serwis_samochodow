#include "serwis_ipc.h"
#include "logger.h"

/**
 * @brief Proces kasjera: odbiera raporty i loguje platnosci.
 */
int main() {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    serwis_log("kasjer", "start");

    while (!serwis_get_pozar()) {
        Raport r{};
        if (serwis_ipc_recv_rap(r) != SERWIS_IPC_OK) break;
        serwis_logf("kasjer", "platnosc id=%d st=%d marka=%c koszt=%d czas=%d",
                    r.id_klienta, r.stanowisko_id, r.s.marka, r.koszt, r.czas);
    }

    serwis_log("kasjer", "koniec");
    serwis_ipc_detach();
    return 0;
}
