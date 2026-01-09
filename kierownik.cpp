// kierownik.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "stanowiska_status.h"

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
    std::cout << "  1 - sygnal1: zamknij stanowisko po biezacej (SIGUSR1)" << std::endl;
    std::cout << "  2 - sygnal2: przyspiesz 50% (SIGUSR2)" << std::endl;
    std::cout << "  3 - sygnal3: normalny (SIGTERM)" << std::endl;
    std::cout << "  4 - sygnal4: pozar (SIGINT + serwis_pozar.flag)" << std::endl;
    std::cout << "  9 - usun pozar.flag" << std::endl;
    std::cout << "  0 - wyjscie" << std::endl;

    while (true) {
        int wybor = -1;
        std::cout << "[kierownik] akcja: ";
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

        if (wybor == 9) {
            serwis_pozar_usun();
            std::cout << "[kierownik] usunieto serwis_pozar.flag" << std::endl;
            continue;
        }

        int stanowisko_id = 0;
        std::cout << "[kierownik] stanowisko 1..8 (dla akcji 1..3), lub 0 dla pozaru: ";
        std::cin >> stanowisko_id;

        if (!std::cin.good()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (wybor == 4) {
            serwis_pozar_ustaw();
            std::cout << "[kierownik] ustawiono serwis_pozar.flag" << std::endl;

            // Dla pozaru: wysylamy SIGINT do wybranego lub wszystkich
            if (stanowisko_id == 0) {
                for (int id = 1; id <= 8; ++id) {
                    int pid = serwis_wczytaj_pid_stanowiska(id);
                    if (pid > 0) kill(pid, SIGINT);
                }
                std::cout << "[kierownik] wyslano SIGINT do wszystkich mechanikow" << std::endl;
            } else {
                int pid = serwis_wczytaj_pid_stanowiska(stanowisko_id);
                if (pid > 0) {
                    kill(pid, SIGINT);
                    std::cout << "[kierownik] wyslano SIGINT do stanowiska=" << stanowisko_id << std::endl;
                } else {
                    std::cout << "[kierownik] brak PID dla stanowiska=" << stanowisko_id << std::endl;
                }
            }
            continue;
        }

        if (stanowisko_id < 1 || stanowisko_id > 8) {
            std::cout << "[kierownik] zly numer stanowiska" << std::endl;
            continue;
        }

        int pid = serwis_wczytaj_pid_stanowiska(stanowisko_id);
        if (pid <= 0) {
            std::cout << "[kierownik] nie moge wczytac PID (mechanik_" << stanowisko_id << "_pid.txt)" << std::endl;
            continue;
        }

        int sig = 0;
        if (wybor == 1) sig = SIGUSR1;
        else if (wybor == 2) sig = SIGUSR2;
        else if (wybor == 3) sig = SIGTERM;
        else {
            std::cout << "[kierownik] zly wybor" << std::endl;
            continue;
        }

        // Akcja 1: ustaw prosbe o zamkniecie (pracownik od razu przestaje wysylac na to stanowisko)
        if (wybor == 1) {
            serwis_stanowisko_ustaw_prosbe_zamkniecia(stanowisko_id);
            std::cout << "[kierownik] ustawiono prosbe zamkniecia dla stanowiska=" << stanowisko_id << std::endl;
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
    std::cout << "[kierownik] pliki flagowe i tak dzialaja, ale sygnaly tylko na Linux" << std::endl;
    return EXIT_SUCCESS;
#endif
}
