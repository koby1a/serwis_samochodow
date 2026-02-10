#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <csignal>
#include <cerrno>
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

union semun { int val; struct semid_ds* buf; unsigned short* array; };

static volatile sig_atomic_t g_stop = 0;
static int g_start_sem = -1;
static pid_t g_parent_pid = -1;

static void on_sig(int) {
    g_stop = 1;
}

static void cleanup_start_sem() {
    if (g_start_sem != -1 && getpid() == g_parent_pid) {
        (void)semctl(g_start_sem, 0, IPC_RMID);
        g_start_sem = -1;
    }
}

int main(int argc, char** argv) {
    g_parent_pid = getpid();
    struct sigaction sa{};
    sa.sa_handler = on_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    (void)sigaction(SIGINT, &sa, nullptr);
    (void)sigaction(SIGTERM, &sa, nullptr);

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
    } else if (scenario == "T2") {
        const char brands[] = {'A','E','I','O','U','Y'};
        for (int i = 0; i < n; ++i) {
            Samochod s{};
            s.marka = brands[serwis_losuj_int(&seed, 0, 5)];
            s.czas_przyjazdu = serwis_time_get();
            s.krytyczna = 1;
            s.krytyczna_typ = serwis_losuj_int(&seed, 1, 3);
            s.czas_naprawy = 0;
            fixed.push_back(s);
        }
    } else if (scenario == "T3") {
        const char brands[] = {'A','E','I','O','U','Y'};
        for (int i = 0; i < n; ++i) {
            Samochod s{};
            s.marka = brands[serwis_losuj_int(&seed, 0, 5)];
            s.czas_przyjazdu = serwis_time_get();
            s.krytyczna = 0;
            s.krytyczna_typ = 0;
            s.czas_naprawy = 0;
            fixed.push_back(s);
        }
    } else if (scenario == "T4") {
        const char brands[] = {'A','E','I','O','U','Y'};
        for (int i = 0; i < n; ++i) {
            Samochod s{};
            s.marka = brands[serwis_losuj_int(&seed, 0, 5)];
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
    int children = 0;
    int start_sem = -1;
    bool handled_a_only = false;

    std::vector<pid_t> a_only_pids;
    if (scenario == "A_ONLY" || scenario == "T2" || scenario == "T3" || scenario == "T4") {
        if (scenario == "A_ONLY") serwis_logf("kierowca", "A_ONLY start: target=%d", n);
        if (scenario == "T2") serwis_logf("kierowca", "T2 start: target=%d", n);
        if (scenario == "T3") serwis_logf("kierowca", "T3 start: target=%d", n);
        if (scenario == "T4") serwis_logf("kierowca", "T4 start: target=%d", n);
        start_sem = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
        if (start_sem == -1) {
            perror("[kierowca] semget start");
        } else {
            semun u{}; u.val = 0;
            if (semctl(start_sem, 0, SETVAL, u) == -1) perror("[kierowca] semctl SETVAL");
        }
        g_start_sem = start_sem;

        size_t i = 0;
        while (i < fixed.size() && sent < n && !serwis_get_pozar() && !g_stop) {
            pid_t pid = fork();
            if (pid == 0) {
                if (start_sem != -1) {
                    struct sembuf op{}; op.sem_num = 0; op.sem_op = -1; op.sem_flg = 0;
                    if (semop(start_sem, &op, 1) == -1) _exit(0);
                }
                (void)serwis_ipc_send_zgl(fixed[i]);
                _exit(0);
            }
            if (pid < 0) {
                perror("[kierowca] fork");
                break;
            }
            children++;
            a_only_pids.push_back(pid);
            if (scenario == "A_ONLY") serwis_logf("kierowca", "A_ONLY fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            if (scenario == "T2") serwis_logf("kierowca", "T2 fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            if (scenario == "T3") serwis_logf("kierowca", "T3 fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            if (scenario == "T4") serwis_logf("kierowca", "T4 fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            ++i;
            ++sent;
        }

        for (int i = 0; i < 100 && !g_stop; ++i) usleep(100000);
        if (!g_stop && start_sem != -1) {
            struct sembuf op{}; op.sem_num = 0; op.sem_op = 1; op.sem_flg = 0;
            for (int i = 0; i < children; ++i) (void)semop(start_sem, &op, 1);
        } else if (g_stop && start_sem != -1) {
            // przerwij semafor, zeby dzieci wyszly z semop
            (void)semctl(start_sem, 0, IPC_RMID);
            g_start_sem = -1;
        }

        handled_a_only = true;
    }

    if (!handled_a_only) {
        for (size_t i = 0; i < fixed.size() && sent < n && !serwis_get_pozar(); ++i, ++sent) {
            if (g_stop) break;
            pid_t pid = fork();
            if (pid == 0) {
                (void)serwis_ipc_send_zgl(fixed[i]);
                _exit(0);
            }
            if (pid > 0) {
                children++;
                // Sprzataj zakonczone dzieci, zeby nie zostawic zombie.
                int status = 0;
                while (waitpid(-1, &status, WNOHANG) > 0) children--;
            }
            serwis_sleep_ms_scaled(sleep_ms, time_scale);
        }
    }

    if (!handled_a_only && burst_mode && !serwis_get_pozar() && !g_stop) {
        const int bursts[] = {4, 10, 2, 7, 1, 6};
        const char brands[] = {'A','E','I','O','U','Y'};
        for (int b = 0; b < (int)(sizeof(bursts)/sizeof(bursts[0])); ++b) {
            int count = bursts[b];
            for (int i = 0; i < count && !serwis_get_pozar(); ++i) {
                if (g_stop) break;
                Samochod s{};
                s.marka = brands[serwis_losuj_int(&seed, 0, 5)];
                s.czas_przyjazdu = serwis_time_get();
                s.krytyczna = 0;
                s.krytyczna_typ = 0;
                s.czas_naprawy = 0;
                pid_t pid = fork();
                if (pid == 0) {
                    (void)serwis_ipc_send_zgl(s);
                    _exit(0);
                }
                if (pid > 0) {
                    children++;
                    int status = 0;
                    while (waitpid(-1, &status, WNOHANG) > 0) children--;
                }
                sent++;
                serwis_sleep_ms_scaled(sleep_ms, time_scale);
            }
            serwis_sleep_ms_scaled(1500, time_scale);
        }
    }

    for (; !handled_a_only && sent < n && !serwis_get_pozar(); ++sent) {
        if (g_stop) break;
        Samochod s{};
        s.marka = (char)('A' + serwis_losuj_int(&seed, 0, 25));
        int sim_t = serwis_time_get();
        int offset = serwis_losuj_int(&seed, -time_offset_range, time_offset_range);
        int t = sim_t + offset;
        if (t < 0) t += 1440;
        if (t >= 1440) t %= 1440;
        s.czas_przyjazdu = t;
        s.krytyczna = 0;
        if (serwis_losuj_int(&seed, 0, 99) < 10) s.krytyczna = 1;
        if (s.krytyczna) s.krytyczna_typ = serwis_losuj_int(&seed, 1, 3);
        else s.krytyczna_typ = 0;
        s.czas_naprawy = 0;

        pid_t pid = fork();
        if (pid == 0) {
            (void)serwis_ipc_send_zgl(s);
            _exit(0);
        }
        if (pid > 0) {
            children++;
            int status = 0;
            while (waitpid(-1, &status, WNOHANG) > 0) children--;
        }
        serwis_sleep_ms_scaled(sleep_ms, time_scale);
    }

    if (g_stop || serwis_get_pozar()) {
        for (pid_t pid : a_only_pids) if (pid > 0) kill(pid, SIGTERM);
    }

    // Doczysc wszystkie pozostale dzieci.
    while (children > 0) {
        int status = 0;
        if (waitpid(-1, &status, 0) > 0) children--;
        else break;
    }
    cleanup_start_sem();

    if (!serwis_get_pozar()) {
        if (workers < 1) workers = 1;
        for (int i = 0; i < workers; ++i) (void)serwis_ipc_send_zgl_shutdown();
    }

    serwis_log("kierowca", "koniec");
    serwis_ipc_detach();
    return 0;
}
