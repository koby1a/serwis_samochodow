#include <iostream>
#include "serwis_ipc.h"
#include "logger.h"

/**
 * @brief Proces kierownika: req_close i pozar.
 */
int main() {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    serwis_log("kierownik", "start");

    while (!serwis_get_pozar()) {
        std::cout << "\n[kierownik] 1) req_close 2) pozar 3) wyjdz\nwybor: ";
        int x = 0;
        if (!(std::cin >> x)) break;

        if (x == 1) {
            int id = 0;
            std::cout << "id (1..8): ";
            std::cin >> id;
            if (id < 1 || id > 8) { std::cout << "bledne id\n"; continue; }
            serwis_req_close(id, 1);
            serwis_logf("kierownik", "req_close id=%d", id);
        } else if (x == 2) {
            serwis_set_pozar(1);
            serwis_log("kierownik", "pozar");
        } else if (x == 3) {
            break;
        }
    }

    serwis_log("kierownik", "koniec");
    serwis_ipc_detach();
    return 0;
}
