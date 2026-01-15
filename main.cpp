// main.cpp
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>

#include "serwis_ipc.h"
#include "stanowiska_status.h"
#include "logger.h"

#if defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>
    #include <sys/wait.h>
    #include <signal.h>
#endif

static void serwis_wyczysc_flagi_start() {
    // Na start czyscimy pozar i statusy stanowisk
    serwis_pozar_usun();
    for (int id = 1; id <= 8; ++id) {
        serwis_stanowisko_usun_zamkniete(id);
        serwis_stanowisko_usun_prosbe_zamkniecia(id);
    }
}

#if defined(__unix__) || defined(__APPLE__)

static pid_t serwis_spawn(const char* prog, const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("[main] fork");
        return -1;
    }
    if (pid == 0) {
        // child
        std::vector<char*> argv;
        argv.reserve(args.size() + 2);
        argv.push_back(const_cast<char*>(prog));
        for (const auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);

        execv(prog, argv.data());
        perror("[main] execv");
        _exit(127);
    }
    return pid;
}

#endif

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    serwis_logger_set_file("raport_symulacji.log");
    serwis_log("main", "start orchestratora");

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[main] blad ipc_init" << std::endl;
        serwis_log("main", "blad ipc_init");
        return EXIT_FAILURE;
    }

    serwis_wyczysc_flagi_start();

#if defined(__unix__) || defined(__APPLE__)
    std::cout << "[main] start procesu orchestratora (Linux/Unix)" << std::endl;

    std::vector<pid_t> dzieci;

    // 8 mechanikow
    for (int id = 1; id <= 8; ++id) {
        std::string prog = "./mechanik";
        std::vector<std::string> args = {"--id", std::to_string(id), "--log", "raport_symulacji.log"};
        pid_t p = serwis_spawn(prog.c_str(), args);
        if (p > 0) dzieci.push_back(p);
    }

    // kasjer
    {
        std::string prog = "./kasjer";
        std::vector<std::string> args = {"--log", "raport_symulacji.log"};
        pid_t p = serwis_spawn(prog.c_str(), args);
        if (p > 0) dzieci.push_back(p);
    }

    // pracownik
    {
        std::string prog = "./pracownik_serwisu";
        std::vector<std::string> args = {"--log", "raport_symulacji.log"};
        pid_t p = serwis_spawn(prog.c_str(), args);
        if (p > 0) dzieci.push_back(p);
    }

    // kierowca (generator) - tutaj ustalasz ilosc aut
    pid_t pid_kierowca = -1;
    {
        std::string prog = "./kierowca";
        std::vector<std::string> args = {"--n", "60", "--seed", "2026", "--sleep_ms", "10", "--log", "raport_symulacji.log"};
        pid_kierowca = serwis_spawn(prog.c_str(), args);
        if (pid_kierowca > 0) dzieci.push_back(pid_kierowca);
    }

    // Czekamy az kierowca sie skonczy
    if (pid_kierowca > 0) {
        int st = 0;
        waitpid(pid_kierowca, &st, 0);
        serwis_log("main", "kierowca zakonczony, finalizuje serwis");
    }

    // Dajemy chwile na obrobke kolejek
    sleep(2);

    // Konczymy cala symulacje "pozar" (ladne domkniecie)
    serwis_pozar_ustaw();
    for (pid_t p : dzieci) {
        if (p > 0) kill(p, SIGINT);
    }

    // Wait na wszystkie dzieci
    for (pid_t p : dzieci) {
        if (p <= 0) continue;
        int st = 0;
        waitpid(p, &st, 0);
    }

    // Wypisz statystyki
    SerwisStatystyki st{};
    if (serwis_stat_pobierz(st) == SERWIS_IPC_OK) {
        serwis_logf("main", "STAT przyjete=%d od_marka=%d od_poza=%d od_oferta=%d brak_st=%d zlecenia=%d naprawy=%d platnosci=%d",
                    st.przyjete_zgloszenia, st.odrzucone_marka, st.odrzucone_poza_godzinami,
                    st.odrzucone_oferta, st.brak_stanowiska, st.wyslane_zlecenia,
                    st.wykonane_naprawy, st.platnosci);
    }

    serwis_ipc_cleanup();
    serwis_ipc_cleanup_all();
    serwis_log("main", "koniec orchestratora");
    return EXIT_SUCCESS;

#else
    std::cout << "[main] wykryto system nie-Unix (np. Windows)" << std::endl;
    std::cout << "[main] fork/exec/wait sa pominiete w tym srodowisku" << std::endl;
    std::cout << "[main] prawdziwa symulacja procesowa dziala na Linux" << std::endl;

    serwis_ipc_cleanup();
    return EXIT_SUCCESS;
#endif
}
