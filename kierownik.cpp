// kierownik.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#if defined(__unix__) || defined(__APPLE__)
    #include <csignal>
    #include <signal.h>
    #include <unistd.h>
#endif

static int serwis_wczytaj_pid_stanowiska(int stanowisko_id) {
    std::string nazwa = "mechanik_" + std::to_string(stanowisko_id) + "_pid.txt";
    std::ifstream f(nazwa.c_str());
    if (!f.is_open()) return -1;
    long long pid = -1;
    f >> pid;
    if (pid <= 0) return -1;
    return static_cast<int>(pid);
}

int main() {
    std::cout << "[kierownik] start" << std::endl;

#if defined(__unix__) || defined(__APPLE__)

    std::cout << "[kierownik] MENU:" << std::endl;
    std::cout << "  wybierz stanowisko 1..8, potem:" << std::endl;
    std::cout << "  1 - sygnal1: zamknij po biezacej (SIGUSR1)" << std::endl;
    std::cout << "  2 - sygnal2: przyspiesz 50% (SIGUSR2)" << std::endl;
    std::cout << "  3 - sygnal3: normalny (SIGTERM)" << std::endl;
    std::cout << "  4 - sygnal4: pozar (SIGINT)" << std::endl;
    std::cout << "  0 - wyjscie" << std::endl;

    while (true) {
        int stanowisko_id = 0;
        std::cout << "[kierownik] stanowisko (1..8): ";
        std::cin >> stanowisko_id;

        if (!std::cin.good()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (stanowisko_id == 0) {
            std::cout << "[kierownik] koniec" << std::endl;
            break;
        }

        if (stanowisko_id < 1 || stanowisko_id > 8) {
            std::cout << "[kierownik] zly numer stanowiska" << std::endl;
            continue;
        }

        int pid = serwis_wczytaj_pid_stanowiska(stanowisko_id);
        if (pid <= 0) {
            std::cout << "[kierownik] nie moge wczytac PID dla stanowiska "
                      << stanowisko_id << " (sprawdz plik mechanik_" << stanowisko_id
                      << "_pid.txt)" << std::endl;
            continue;
        }

        int wybor = -1;
        std::cout << "[kierownik] akcja (1..4, 0=wyjscie): ";
        std::cin >> wybor;

        if (!std::cin.good()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (wybor == 0) {
            std::cout << "[kierownik] koniec" << std::endl;
            break;
        }

        int sig = 0;
        if (wybor == 1) sig = SIGUSR1;
        else if (wybor == 2) sig = SIGUSR2;
        else if (wybor == 3) sig = SIGTERM;
        else if (wybor == 4) sig = SIGINT;
        else {
            std::cout << "[kierownik] zly wybor" << std::endl;
            continue;
        }

        if (kill(pid, sig) == -1) {
            perror("[kierownik] kill()");
        } else {
            std::cout << "[kierownik] wyslano sygnal do stanowiska=" << stanowisko_id
                      << ", pid=" << pid << std::endl;
        }

        sleep(1);
    }

    return EXIT_SUCCESS;

#else
    std::cout << "[kierownik] system nie-Unix - brak kill/sygnalow (stub)" << std::endl;
    return EXIT_SUCCESS;
#endif
}
