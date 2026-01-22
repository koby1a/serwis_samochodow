<<<<<<< HEAD
=======
/** @file main.cpp */
>>>>>>> 49aa6d4 (v20)
#include <vector>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include "serwis_ipc.h"
#include "logger.h"

<<<<<<< HEAD
/**
 * @brief Uruchamia proces przez fork+execv.
 */
static pid_t spawnp(const char* prog, const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid < 0) { perror("[main] fork"); return -1; }
    if (pid == 0) {
        std::vector<char*> argv;
        argv.reserve(args.size() + 2);
=======
static pid_t spawnp(const char* prog, const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> argv;
>>>>>>> 49aa6d4 (v20)
        argv.push_back(const_cast<char*>(prog));
        for (const auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);
        execv(prog, argv.data());
<<<<<<< HEAD
        perror("[main] execv");
        _exit(127);
=======
        _exit(1);
>>>>>>> 49aa6d4 (v20)
    }
    return pid;
}

int main() {
    serwis_logger_set_file("raport_symulacji.log");
    serwis_logger_reset_file();
<<<<<<< HEAD
    serwis_log("main", "start");

    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    serwis_set_pozar(0);
=======
    
    if (serwis_ipc_init() != SERWIS_IPC_OK) return 1;

    serwis_set_pozar(0);
    const int sim_start_min = 8 * 60;
    const int sim_tick_ms = 200;
    const int sim_step_min = 1;
    serwis_time_set(sim_start_min);
>>>>>>> 49aa6d4 (v20)
    for (int i = 1; i <= 8; ++i) {
        serwis_station_set_closed(i, 0);
        serwis_req_close(i, 0);
        serwis_station_set_busy(i, 0, '-', 0, 0, 0);
    }

    std::vector<pid_t> kids;
<<<<<<< HEAD

    kids.push_back(spawnp("./dashboard", {}));
    kids.push_back(spawnp("./kasjer", {}));
    kids.push_back(spawnp("./pracownik_serwisu", {}));
    for (int id = 1; id <= 8; ++id) {
        kids.push_back(spawnp("./mechanik", {"--id", std::to_string(id)}));
    }
    kids.push_back(spawnp("./kierownik", {}));

    pid_t pid_kierowca = spawnp("./kierowca", {"--n", "200", "--seed", "2026", "--sleep_ms", "5"});
    kids.push_back(pid_kierowca);

    int stc = 0;
    waitpid(pid_kierowca, &stc, 0);

    sleep(2);
    serwis_set_pozar(1);
    for (pid_t p : kids) if (p > 0) kill(p, SIGINT);

    for (pid_t p : kids) {
        if (p <= 0) continue;
        int stw = 0;
        waitpid(p, &stw, 0);
    }

    serwis_ipc_detach();
    serwis_ipc_cleanup_all();
    serwis_log("main", "koniec");
=======
    kids.push_back(spawnp("./dashboard", {}));
    kids.push_back(spawnp("./kasjer", {}));
    kids.push_back(spawnp("./pracownik_serwisu", {}));
    for (int id = 1; id <= 8; ++id) 
        kids.push_back(spawnp("./mechanik", {"--id", std::to_string(id)}));
    kids.push_back(spawnp("./kierownik", {}));
    kids.push_back(spawnp("./kierowca", {"--n", "200", "--sleep_ms", "200"}));

    int sim_time = sim_start_min;
    // Czekaj na zakończenie kierowcy lub sygnał pożaru
    while (true) {
        int status;
        pid_t p = waitpid(-1, &status, WNOHANG);
        if (serwis_get_pozar()) break;
        // Jeśli kierowca (ostatni na liście) skończył, też kończymy
        (void)p;
        usleep((useconds_t)sim_tick_ms * 1000u);
        sim_time += sim_step_min;
        if (sim_time >= 1440) sim_time %= 1440;
        serwis_time_set(sim_time);
    }

    serwis_log("main", "zakonczenie symulacji - czyszczenie");
    for (pid_t k : kids) kill(k, SIGINT);
    
    // Daj procesom chwilę na odpięcie SHM
    sleep(1); 
    serwis_ipc_cleanup_all();
    
>>>>>>> 49aa6d4 (v20)
    return 0;
}
