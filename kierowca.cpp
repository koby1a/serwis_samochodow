#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
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

static volatile sig_atomic_t g_stop = 0;
static volatile sig_atomic_t g_child_stop = 0;
static int g_start_fd_r = -1;
static int g_start_fd_w = -1;
static pid_t g_parent_pid = -1;

static void on_sig(int) {
    g_stop = 1;
}

static void on_child_sig(int) {
    g_child_stop = 1;
}

static void cleanup_start_pipe() {
    if (getpid() == g_parent_pid) {
        if (g_start_fd_r != -1) close(g_start_fd_r);
        if (g_start_fd_w != -1) close(g_start_fd_w);
        g_start_fd_r = -1;
        g_start_fd_w = -1;
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
    if (scenario == "T1") {
        for (int i = 0; i < n; ++i) {
            Samochod s{};
            s.marka = 'A';
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
    }

    int sent = 0;
    int children = 0;
    int start_r = -1;
    int start_w = -1;
    bool handled_sync = false;

    std::vector<pid_t> sync_pids;
    if (scenario == "T1" || scenario == "T2" || scenario == "T3" || scenario == "T4") {
        if (scenario == "T1") serwis_logf("kierowca", "T1 start: target=%d", n);
        if (scenario == "T2") serwis_logf("kierowca", "T2 start: target=%d", n);
        if (scenario == "T3") serwis_logf("kierowca", "T3 start: target=%d", n);
        if (scenario == "T4") serwis_logf("kierowca", "T4 start: target=%d", n);
        int fds[2] = {-1, -1};
        if (pipe(fds) != 0) {
            perror("[kierowca] pipe start");
        } else {
            start_r = fds[0];
            start_w = fds[1];
        }
        g_start_fd_r = start_r;
        g_start_fd_w = start_w;

        size_t i = 0;
        while (i < fixed.size() && sent < n && !serwis_get_pozar() && !g_stop) {
            pid_t pid = fork();
            if (pid == 0) {
                struct sigaction sa_child{};
                sa_child.sa_handler = on_child_sig;
                sigemptyset(&sa_child.sa_mask);
                sa_child.sa_flags = 0;
                (void)sigaction(SIGTERM, &sa_child, nullptr);
                (void)sigaction(SIGINT, &sa_child, nullptr);

                if (start_r != -1) {
                    if (start_w != -1) close(start_w);
                    char b = 0;
                    while (true) {
                        ssize_t r = read(start_r, &b, 1);
                        if (r == 1) break;
                        if (r == 0) _exit(0);
                        if (errno == EINTR) continue;
                        _exit(0);
                    }
                }
                fixed[i].pid_klienta = (int)getpid();
                (void)serwis_ipc_send_zgl(fixed[i]);
                while (!g_child_stop) pause();
                _exit(0);
            }
            if (pid < 0) {
                perror("[kierowca] fork");
                break;
            }
            children++;
            sync_pids.push_back(pid);
            if (scenario == "T1") serwis_logf("kierowca", "T1 fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            if (scenario == "T2") serwis_logf("kierowca", "T2 fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            if (scenario == "T3") serwis_logf("kierowca", "T3 fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            if (scenario == "T4") serwis_logf("kierowca", "T4 fork pid=%d idx=%d/%d", (int)pid, sent + 1, n);
            ++i;
            ++sent;
        }

        if (!g_stop) sleep(10);
        if (!g_stop && start_w != -1) {
            for (int i = 0; i < children; ++i) (void)write(start_w, "x", 1);
            close(start_w);
            start_w = -1;
            g_start_fd_w = -1;
        } else if (g_stop && start_w != -1) {
            // zamknij pipe, zeby dzieci wyszly z read
            close(start_w);
            start_w = -1;
            g_start_fd_w = -1;
        }

        handled_sync = true;
    }

    if (!handled_sync) {
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

    for (; !handled_sync && sent < n && !serwis_get_pozar(); ++sent) {
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
        for (pid_t pid : sync_pids) if (pid > 0) kill(pid, SIGTERM);
    }

    // Doczysc wszystkie pozostale dzieci.
    while (children > 0) {
        int status = 0;
        if (waitpid(-1, &status, 0) > 0) children--;
        else break;
    }
    cleanup_start_pipe();

    if (!serwis_get_pozar()) {
        if (workers < 1) workers = 1;
        for (int i = 0; i < workers; ++i) (void)serwis_ipc_send_zgl_shutdown();
    }

    serwis_log("kierowca", "koniec");
    serwis_ipc_detach();
    return 0;
}
