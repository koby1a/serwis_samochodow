/** @file dashboard.cpp */

#include <iostream>
#include <unistd.h>
#include "serwis_ipc.h"
#include "ui.h"

/** @brief Zwraca kolor dla marki. */
static const char* marka_color(char m) {
    switch (m) {
        case 'A': return ui_cyan();
        case 'E': return ui_green();
        case 'I': return ui_blue();
        case 'O': return ui_magenta();
        case 'U': return ui_yellow();
        case 'Y': return ui_yellow();
        default:  return ui_gray();
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cout.setf(std::ios::unitbuf);

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "dashboard: ipc_init error\n";
        return 1;
    }

    ui_clear();

    while (!serwis_get_pozar()) {
        SerwisStatystyki s{};
        if (serwis_stat_get(s) != SERWIS_IPC_OK) {
            std::cout << "dashboard: stat_get error\n";
            usleep(200000);
            continue;
        }

        ui_clear();
        ui_home();
        std::cout << ui_cyan() << "=== DASHBOARD SERWISU ===" << ui_reset() << "\n\n";
        int h = (s.sim_time_min / 60) % 24;
        int m = s.sim_time_min % 60;
        std::cout << ui_yellow() << "CZAS SYMULACJI: "
                  ;
        if (h < 10) std::cout << "0";
        std::cout << h << ":";
        if (m < 10) std::cout << "0";
        std::cout << m << ui_reset() << "\n\n";

        for (int id = 1; id <= 8; ++id) {
            const auto& st = s.st[id];

            std::cout << "Stanowisko " << id << ": ";

            if (st.zamkniete) std::cout << ui_red() << "ZAMKNIETE" << ui_reset();
            else if (st.zajete) std::cout << ui_green() << "ZAJETE" << ui_reset();
            else std::cout << ui_gray() << "WOLNE" << ui_reset();

            std::cout << " | obsluzone=" << st.obsluzone;
            std::cout << " | marka=" << marka_color(st.marka) << st.marka << ui_reset();

            if (st.krytyczna) std::cout << " " << ui_red() << "[KRYTYCZNA]" << ui_reset();
            if (st.dodatkowe) std::cout << " " << ui_yellow() << "[DODATKOWE]" << ui_reset();

            std::cout << " | tryb=";
            if (st.tryb == SERWIS_TRYB_PRZYSPIESZONY) std::cout << ui_yellow() << "PRZYSPIESZONY" << ui_reset();
            else std::cout << "NORMALNY" << ui_reset();

            if (s.req_close[id]) std::cout << " " << ui_magenta() << "[REQ_CLOSE]" << ui_reset();
            std::cout << "\n";
        }

        std::cout << "\nCtrl+C / pozar zamyka symulacje.\n";
        usleep(200000);
    }

    serwis_ipc_detach();
    return 0;
}
