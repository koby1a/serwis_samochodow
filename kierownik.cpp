// kierownik.cpp
#include <iostream>
#include <fstream>
#include <cstdlib>

#if defined(__unix__) || defined(__APPLE__)
    #include <csignal>   // SIGUSR1, SIGUSR2, SIGTERM, SIGINT
    #include <sys/types.h>
    #include <signal.h>  // kill
    #include <unistd.h>  // sleep
#endif

static int serwis_wczytaj_pid_z_pliku(const char* nazwa_pliku) {
    std::ifstream f(nazwa_pliku);
    if (!f.is_open()) {
        return -1;
    }
    long long pid = -1;
    f >> pid;
    if (pid <= 0) {
        return -1;
    }
    return static_cast<int>(pid);
}

int main() {
    std::cout << "[kierownik] start procesu kierownika" << std::endl;

#if defined(__unix__) || defined(__APPLE__)
    int pid_mechanik = serwis_wczytaj_pid_z_pliku("mechanik_pid.txt");
    if (pid_mechanik <= 0) {
        std::cerr << "[kierownik] nie moge wczytac PID z mechanik_pid.txt" << std::endl;
        std::cerr << "[kierownik] uruchom najpierw mechanika na Linuxie" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[kierownik] PID mechanika = " << pid_mechanik << std::endl;

    std::cout << "[kierownik] MENU:" << std::endl;
    std::cout << "  1 - sygnal1: zamknij stanowisko po biezacej naprawie (SIGUSR1)" << std::endl;
    std::cout << "  2 - sygnal2: przyspiesz o 50% (SIGUSR2)" << std::endl;
    std::cout << "  3 - sygnal3: powrot do normalnego (SIGTERM)" << std::endl;
    std::cout << "  4 - sygnal4: POZAR, natychmiast stop (SIGINT)" << std::endl;
    std::cout << "  0 - wyjscie" << std::endl;

    while (true) {
        std::cout << "[kierownik] wybierz akcje: ";
        int wybor = -1;
        std::cin >> wybor;

        if (!std::cin.good()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (wybor == 0) {
            std::cout << "[kierownik] koniec pracy kierownika" << std::endl;
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

        if (kill(pid_mechanik, sig) == -1) {
            perror("[kierownik] blad kill()");
        } else {
            std::cout << "[kierownik] wyslano sygnal do mechanika" << std::endl;
        }

        // Mala przerwa zeby logi byly czytelne
        sleep(1);
    }

    return EXIT_SUCCESS;

#else
    std::cout << "[kierownik] system nie-Unix - kill/sygnaly sa niedostepne (stub)" << std::endl;
    std::cout << "[kierownik] uruchom to na Linuxie, wtedy bedzie prawdziwe sterowanie" << std::endl;
    return EXIT_SUCCESS;
#endif
}
