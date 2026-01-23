/** @file main.cpp */
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include "serwis_ipc.h"
#include "logger.h"

static volatile sig_atomic_t g_stop = 0;

static void on_sig(int sig) {
    if (sig == SIGINT || sig == SIGTERM) g_stop = 1;
}

static pid_t spawnp(const char* prog, const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(prog));
        for (const auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);
        execv(prog, argv.data());
        _exit(1);
    }
    return pid;
}

struct SimConfig {
    int n = 200;
    int sleep_ms = 200;
    int seed = 2026;
    int sim_start_min = 8 * 60;
    int sim_tick_ms = 200;
    int sim_step_min = 1;
    int time_offset_range = 180;
    int tp = 8 * 60;
    int tk = 16 * 60;
    int t1 = 60;
    int k1 = 3;
    int k2 = 5;
    std::string scenario = "default";
};

static int argi(int argc, char** argv, const std::string& k, int d) {
    for (int i = 1; i + 1 < argc; ++i) if (k == argv[i]) return std::atoi(argv[i + 1]);
    return d;
}

static std::string args(int argc, char** argv, const std::string& k, const std::string& d) {
    for (int i = 1; i + 1 < argc; ++i) if (k == argv[i]) return argv[i + 1];
    return d;
}

static void apply_kv(SimConfig& c, const std::string& key, const std::string& val) {
    if (key == "n") c.n = std::atoi(val.c_str());
    else if (key == "sleep_ms") c.sleep_ms = std::atoi(val.c_str());
    else if (key == "seed") c.seed = std::atoi(val.c_str());
    else if (key == "sim_start_min") c.sim_start_min = std::atoi(val.c_str());
    else if (key == "sim_tick_ms") c.sim_tick_ms = std::atoi(val.c_str());
    else if (key == "sim_step_min") c.sim_step_min = std::atoi(val.c_str());
    else if (key == "time_offset_range") c.time_offset_range = std::atoi(val.c_str());
    else if (key == "tp") c.tp = std::atoi(val.c_str());
    else if (key == "tk") c.tk = std::atoi(val.c_str());
    else if (key == "t1") c.t1 = std::atoi(val.c_str());
    else if (key == "k1") c.k1 = std::atoi(val.c_str());
    else if (key == "k2") c.k2 = std::atoi(val.c_str());
    else if (key == "scenario") c.scenario = val;
}

static void load_config(const std::string& path, SimConfig& c) {
    if (path.empty()) return;
    std::ifstream f(path);
    if (!f) {
        std::cerr << "main: nie mozna otworzyc configu: " << path << "\n";
        return;
    }
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string key, val;
        if (std::getline(ss, key, '=') && std::getline(ss, val)) {
            if (!key.empty() && !val.empty()) apply_kv(c, key, val);
        }
    }
}

int main(int argc, char** argv) {
    serwis_logger_set_file("raport_symulacji.log");
    serwis_logger_reset_file();

    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    struct sigaction sa{};
    sa.sa_handler = on_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, nullptr) == -1) perror("[main] sigaction SIGINT");
    if (sigaction(SIGTERM, &sa, nullptr) == -1) perror("[main] sigaction SIGTERM");

    SimConfig cfg;
    load_config(args(argc, argv, "--config", ""), cfg);
    cfg.n = argi(argc, argv, "--n", cfg.n);
    cfg.sleep_ms = argi(argc, argv, "--sleep_ms", cfg.sleep_ms);
    cfg.seed = argi(argc, argv, "--seed", cfg.seed);
    cfg.sim_start_min = argi(argc, argv, "--sim_start_min", cfg.sim_start_min);
    cfg.sim_tick_ms = argi(argc, argv, "--sim_tick_ms", cfg.sim_tick_ms);
    cfg.sim_step_min = argi(argc, argv, "--sim_step_min", cfg.sim_step_min);
    cfg.time_offset_range = argi(argc, argv, "--time_offset_range", cfg.time_offset_range);
    cfg.tp = argi(argc, argv, "--tp", cfg.tp);
    cfg.tk = argi(argc, argv, "--tk", cfg.tk);
    cfg.t1 = argi(argc, argv, "--t1", cfg.t1);
    cfg.k1 = argi(argc, argv, "--k1", cfg.k1);
    cfg.k2 = argi(argc, argv, "--k2", cfg.k2);
    cfg.scenario = args(argc, argv, "--scenario", cfg.scenario);

    serwis_set_pozar(0);
    serwis_time_set(cfg.sim_start_min);
    for (int i = 1; i <= 8; ++i) {
        serwis_station_set_closed(i, 0);
        serwis_req_close(i, 0);
        serwis_station_set_busy(i, 0, '-', 0, 0, 0);
    }

    std::vector<pid_t> kids;
    kids.push_back(spawnp("./dashboard", {}));
    kids.push_back(spawnp("./kasjer", {}));
    kids.push_back(spawnp("./pracownik_serwisu", {
        "--tp", std::to_string(cfg.tp),
        "--tk", std::to_string(cfg.tk),
        "--t1", std::to_string(cfg.t1),
        "--k1", std::to_string(cfg.k1),
        "--k2", std::to_string(cfg.k2)
    }));
    for (int id = 1; id <= 8; ++id)
        kids.push_back(spawnp("./mechanik", {"--id", std::to_string(id)}));
    kids.push_back(spawnp("./kierownik", {}));
    pid_t kierowca_pid = spawnp("./kierowca", {
        "--n", std::to_string(cfg.n),
        "--sleep_ms", std::to_string(cfg.sleep_ms),
        "--seed", std::to_string(cfg.seed),
        "--time_offset_range", std::to_string(cfg.time_offset_range),
        "--scenario", cfg.scenario
    });
    kids.push_back(kierowca_pid);

    int sim_time = cfg.sim_start_min;
    while (true) {
        int status;
        pid_t p = waitpid(-1, &status, WNOHANG);
        if (g_stop) {
            serwis_set_pozar(1);
            break;
        }
        if (serwis_get_pozar()) break;
        if (p == kierowca_pid) kierowca_pid = -1;
        usleep((useconds_t)((long long)cfg.sim_tick_ms * 1000LL));
        sim_time += cfg.sim_step_min;
        if (sim_time >= 1440) sim_time %= 1440;
        serwis_time_set(sim_time);
    }

    serwis_log("main", "zakonczenie symulacji - czyszczenie");
    serwis_set_pozar(1);
    for (pid_t k : kids) if (k > 0) kill(k, SIGINT);

    int alive = 0;
    for (pid_t k : kids) if (k > 0) alive++;

    auto reap = [&]() {
        int status;
        pid_t p;
        while ((p = waitpid(-1, &status, WNOHANG)) > 0) {
            for (pid_t& k : kids) {
                if (k == p) {
                    k = -1;
                    alive--;
                    break;
                }
            }
        }
    };

    for (int i = 0; i < 40 && alive > 0; ++i) {
        reap();
        usleep((useconds_t)50000);
    }

    if (alive > 0) {
        for (pid_t k : kids) if (k > 0) kill(k, SIGKILL);
        for (int i = 0; i < 20 && alive > 0; ++i) {
            reap();
            usleep((useconds_t)50000);
        }
    }

    serwis_ipc_cleanup_all();
    return 0;
}
