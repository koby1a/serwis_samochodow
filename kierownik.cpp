#include <iostream>
#include <csignal>
#include <unistd.h>
#include "serwis_ipc.h"
#include "logger.h"

/**
 * @brief Proces kierownika: sygnaly do mechanikow i pozar.
 */
int main() {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    serwis_log("kierownik", "start");

    while (!serwis_get_pozar()) {
        std::cout << "\n[kierownik] 1) zamknij stanowisko 2) przyspiesz 3) przywroc 4) pozar 5) wyjdz\nwybor: ";
        int x = 0;
        if (!(std::cin >> x)) break;

        if (x == 1) {
            int id = 0;
            std::cout << "id (1..8): ";
            std::cin >> id;
            if (id < 1 || id > 8) { std::cout << "bledne id\n"; continue; }
            serwis_req_close(id, 1);
            int pid = serwis_station_get_pid(id);
            if (pid > 0) {
                kill(pid, SIGUSR1);
                serwis_logf("kierownik", "sig_close id=%d pid=%d", id, pid);
            } else {
                std::cout << "brak pid dla stanowiska\n";
            }
        } else if (x == 2 || x == 3) {
            int id = 0;
            std::cout << "id (1..8): ";
            std::cin >> id;
            if (id < 1 || id > 8) { std::cout << "bledne id\n"; continue; }
            int pid = serwis_station_get_pid(id);
            if (pid <= 0) { std::cout << "brak pid dla stanowiska\n"; continue; }
            if (x == 2) {
                kill(pid, SIGUSR2);
                serwis_logf("kierownik", "sig_przyspiesz id=%d pid=%d", id, pid);
            } else {
                kill(pid, SIGTERM);
                serwis_logf("kierownik", "sig_przywroc id=%d pid=%d", id, pid);
            }
        } else if (x == 4) {
            serwis_set_pozar(1);
            serwis_log("kierownik", "pozar");
        } else if (x == 5) {
            break;
        }
    }

    serwis_log("kierownik", "koniec");
    serwis_ipc_detach();
    return 0;
}
