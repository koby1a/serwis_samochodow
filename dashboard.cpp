/** @file dashboard.cpp */

#include <iostream>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include "serwis_ipc.h"
#include "ui.h"
#include "time_scale.h"

/** @brief Zwraca kolor dla marki. */
static const char* kolor_marki(char m) {
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

static int argi(int argc, char** argv, const char* k, int d) {
    for (int i = 1; i + 1 < argc; ++i) if (std::string(argv[i]) == k) return std::atoi(argv[i + 1]);
    return d;
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::cout.setf(std::ios::unitbuf);
    int time_scale = argi(argc, argv, "--time_scale", 10);

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "dashboard: ipc_init error\n";
        return 1;
    }
    serwis_time_scale_set(time_scale);

    ui_clear();

    while (!serwis_get_pozar()) {
        SerwisStatystyki s{};
        if (serwis_stat_get(s) != SERWIS_IPC_OK) {
            std::cout << "dashboard: stat_get error\n";
            serwis_sleep_us_scaled(200000, time_scale);
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

        SerwisQueueCounts qc{};
        if (serwis_ipc_get_queue_counts(qc) == SERWIS_IPC_OK) {
            std::cout << ui_gray() << "KOLEJKI" << ui_reset()
                      << " | zgl=" << qc.zgl
                      << " | zlec=" << qc.zlec
                      << " | rap=" << qc.rap
                      << " | kasa=" << qc.kasa
                      << " | ext_req=" << qc.ext_req
                      << " | ext_resp=" << qc.ext_resp
                      << "\n\n";
        } else {
            std::cout << ui_gray() << "KOLEJKI" << ui_reset()
                      << " | zgl=? | zlec=? | rap=? | kasa=? | ext_req=? | ext_resp=?\n\n";
        }

        for (int id = 1; id <= 8; ++id) {
            const auto& st = s.st[id];

            std::cout << "Stanowisko " << id << ": ";

            if (st.zamkniete) std::cout << ui_red() << "ZAMKNIETE" << ui_reset();
            else if (st.zajete) std::cout << ui_green() << "ZAJETE" << ui_reset();
            else std::cout << ui_gray() << "WOLNE" << ui_reset();

            std::cout << " | obsluzone=" << st.obsluzone;
            std::cout << " | marka=" << kolor_marki(st.marka) << st.marka << ui_reset();

            if (st.krytyczna) std::cout << " " << ui_red() << "[KRYTYCZNA]" << ui_reset();
            if (st.dodatkowe) std::cout << " " << ui_yellow() << "[DODATKOWE]" << ui_reset();

            std::cout << " | tryb=";
            if (st.tryb == SERWIS_TRYB_PRZYSPIESZONY) std::cout << ui_yellow() << "PRZYSPIESZONY" << ui_reset();
            else std::cout << "NORMALNY" << ui_reset();

            if (s.req_close[id]) std::cout << " " << ui_magenta() << "[REQ_CLOSE]" << ui_reset();
            std::cout << "\n";
        }

        std::cout << "\nCtrl+C / pozar zamyka symulacje.\n";
        serwis_sleep_us_scaled(200000, time_scale);
    }

    serwis_ipc_detach();
    return 0;
}
