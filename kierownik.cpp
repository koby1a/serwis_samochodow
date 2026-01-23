#include <iostream>
#include <csignal>
#include <unistd.h>
#include "serwis_ipc.h"
#include "logger.h"

/**
 * @brief Proces kierownika: sygnaly do mechanikow i pozar.
 */
static int read_station_id() {
    int id = 0;
    std::cout << "id (1..8): ";
    if (!(std::cin >> id)) return -1;
    if (id < 1 || id > 8) {
        std::cout << "bledne id\n";
        return -1;
    }
    return id;
}

static void print_menu() {
    std::cout
        << "\n[kierownik] MENU\n"
        << "  Stanowiska:\n"
        << "    1) zamknij stanowisko\n"
        << "    2) otworz stanowisko\n"
        << "  Tryb pracy:\n"
        << "    3) ustaw PRZYSPIESZONY\n"
        << "    4) ustaw NORMALNY\n"
        << "  Inne:\n"
        << "    5) pozar\n"
        << "    6) wyjdz\n"
        << "wybor: ";
}

int main() {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    serwis_log("kierownik", "start");

    while (!serwis_get_pozar()) {
        print_menu();
        int x = 0;
        if (!(std::cin >> x)) break;

        if (x == 1) {
            int id = read_station_id();
            if (id == -1) continue;
            serwis_req_close(id, 1);
            int pid = serwis_station_get_pid(id);
            if (pid > 0) {
                kill(pid, SIGUSR1);
                serwis_logf("kierownik", "sig_close id=%d pid=%d", id, pid);
            } else {
                std::cout << "brak pid dla stanowiska\n";
            }
        } else if (x == 2) {
            int id = read_station_id();
            if (id == -1) continue;
            serwis_station_set_closed(id, 0);
            serwis_req_close(id, 0);
            serwis_logf("kierownik", "open id=%d", id);
        } else if (x == 3 || x == 4) {
            int id = read_station_id();
            if (id == -1) continue;
            int pid = serwis_station_get_pid(id);
            if (pid <= 0) { std::cout << "brak pid dla stanowiska\n"; continue; }
            if (x == 3) {
                kill(pid, SIGUSR2);
                serwis_logf("kierownik", "sig_przyspiesz id=%d pid=%d", id, pid);
            } else {
                kill(pid, SIGTERM);
                serwis_logf("kierownik", "sig_przywroc id=%d pid=%d", id, pid);
            }
        } else if (x == 5) {
            serwis_set_pozar(1);
            serwis_log("kierownik", "pozar");
        } else if (x == 6) {
            break;
        }
    }

    serwis_log("kierownik", "koniec");
    serwis_ipc_detach();
    return 0;
}
