#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include "serwis_ipc.h"
#include "model.h"
#include "logger.h"
#include "time_scale.h"

/**
 * @brief Pobiera int z argv.
 */
static int argi(int argc, char** argv, const std::string& k, int d) {
    for (int i = 1; i + 1 < argc; ++i) if (k == argv[i]) return std::atoi(argv[i + 1]);
    return d;
}

static std::string args(int argc, char** argv, const std::string& k, const std::string& d) {
    for (int i = 1; i + 1 < argc; ++i) if (k == argv[i]) return argv[i + 1];
    return d;
}

int main(int argc, char** argv) {
    serwis_logger_set_file("raport_symulacji.log");
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    int n = argi(argc, argv, "--n", 200);
    int sleep_ms = argi(argc, argv, "--sleep_ms", 5);
    int time_offset_range = argi(argc, argv, "--time_offset_range", 180);
    int time_scale = argi(argc, argv, "--time_scale", 10);
    int workers = argi(argc, argv, "--workers", 3);
    serwis_time_scale_set(time_scale);
    unsigned int seed = (unsigned int)argi(argc, argv, "--seed", 2026);
    std::string scenario = args(argc, argv, "--scenario", "default");

    if (n < 1) n = 1;
    if (n > 50000) n = 50000;
    if (sleep_ms < 0) sleep_ms = 0;
    if (sleep_ms > 5000) sleep_ms = 5000;
    if (time_offset_range < 0) time_offset_range = 0;
    if (time_offset_range > 720) time_offset_range = 720;

    serwis_logf("kierowca", "start n=%d seed=%u scen=%s", n, seed, scenario.c_str());

    std::vector<Samochod> fixed;
    bool burst_mode = false;
    if (scenario == "T1") {
        const char brands[] = {'A','E','I','O','U','U','U','Y','Y','Y','B','C','Z'};
        for (char b : brands) {
            Samochod s{};
            s.marka = b;
            s.czas_przyjazdu = serwis_time_get();
            s.krytyczna = 0;
            s.krytyczna_typ = 0;
            s.czas_naprawy = 0;
            fixed.push_back(s);
        }
    } else if (scenario == "A_ONLY") {
        for (int i = 0; i < n; ++i) {
            Samochod s{};
            s.marka = 'A';
            s.czas_przyjazdu = serwis_time_get();
            s.krytyczna = 0;
            s.krytyczna_typ = 0;
            s.czas_naprawy = 0;
            fixed.push_back(s);
        }
    } else if (scenario == "TIME_GATES") {
        Samochod s1{}; s1.marka='A'; s1.czas_przyjazdu=400; s1.krytyczna=1; s1.krytyczna_typ=1; fixed.push_back(s1);
        Samochod s2{}; s2.marka='E'; s2.czas_przyjazdu=450; s2.krytyczna=0; s2.krytyczna_typ=0; fixed.push_back(s2); // do otwarcia = 30
        Samochod s3{}; s3.marka='I'; s3.czas_przyjazdu=460; s3.krytyczna=0; s3.krytyczna_typ=0; fixed.push_back(s3); // do otwarcia = 20
        Samochod s4{}; s4.marka='O'; s4.czas_przyjazdu=490; s4.krytyczna=0; s4.krytyczna_typ=0; fixed.push_back(s4);
        Samochod s5{}; s5.marka='U'; s5.czas_przyjazdu=970; s5.krytyczna=0; s5.krytyczna_typ=0; fixed.push_back(s5);
        Samochod s6{}; s6.marka='Y'; s6.czas_przyjazdu=990; s6.krytyczna=1; s6.krytyczna_typ=2; fixed.push_back(s6);
        if ((int)fixed.size() < n) {
            int extra = n - (int)fixed.size();
            for (int i = 0; i < extra; ++i) {
                Samochod s{};
                s.marka = (char)('A' + serwis_losuj_int(&seed, 0, 25));
                s.czas_przyjazdu = serwis_time_get();
                s.krytyczna = 0;
                s.krytyczna_typ = 0;
                s.czas_naprawy = 0;
                fixed.push_back(s);
            }
        }
    } else if (scenario == "BURST_K1K2") {
        burst_mode = true;
    }

    int sent = 0;
    for (size_t i = 0; i < fixed.size() && sent < n && !serwis_get_pozar(); ++i, ++sent) {
        if (serwis_ipc_send_zgl(fixed[i]) != SERWIS_IPC_OK) break;
        serwis_sleep_ms_scaled(sleep_ms, time_scale);
    }

    if (burst_mode && !serwis_get_pozar()) {
        const int bursts[] = {4, 10, 2, 7, 1, 6};
        const char brands[] = {'A','E','I','O','U','Y'};
        for (int b = 0; b < (int)(sizeof(bursts)/sizeof(bursts[0])); ++b) {
            int count = bursts[b];
            for (int i = 0; i < count && !serwis_get_pozar(); ++i) {
                Samochod s{};
                s.marka = brands[serwis_losuj_int(&seed, 0, 5)];
                s.czas_przyjazdu = serwis_time_get();
                s.krytyczna = 0;
                s.krytyczna_typ = 0;
                s.czas_naprawy = 0;
                if (serwis_ipc_send_zgl(s) != SERWIS_IPC_OK) { b = 999; break; }
                sent++;
                serwis_sleep_ms_scaled(sleep_ms, time_scale);
            }
            serwis_sleep_ms_scaled(1500, time_scale);
        }
    }

    for (; sent < n && !serwis_get_pozar(); ++sent) {
        Samochod s{};
        s.marka = (char)('A' + serwis_losuj_int(&seed, 0, 25));
        int sim_t = serwis_time_get();
        int offset = serwis_losuj_int(&seed, -time_offset_range, time_offset_range);
        int t = sim_t + offset;
        if (t < 0) t += 1440;
        if (t >= 1440) t %= 1440;
        s.czas_przyjazdu = t;
        s.krytyczna = (serwis_losuj_int(&seed, 0, 99) < 10) ? 1 : 0;
        s.krytyczna_typ = s.krytyczna ? serwis_losuj_int(&seed, 1, 3) : 0;
        s.czas_naprawy = 0;

        if (serwis_ipc_send_zgl(s) != SERWIS_IPC_OK) break;
        serwis_sleep_ms_scaled(sleep_ms, time_scale);
    }

    if (!serwis_get_pozar()) {
        if (workers < 1) workers = 1;
        for (int i = 0; i < workers; ++i) (void)serwis_ipc_send_zgl_shutdown();
    }

    serwis_log("kierowca", "koniec");
    serwis_ipc_detach();
    return 0;
}
