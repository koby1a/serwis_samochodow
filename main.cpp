// main.cpp
#include <iostream>
#include <cstdlib>

#include "model.h"
#include "serwis_ipc.h"

// Sekcja specyficzna dla systemow Unix (Linux, macOS)
#if defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>     // fork, execl
    #include <sys/types.h>  // pid_t
    #include <sys/wait.h>   // waitpid
    #include <errno.h>      // errno
    #include <cstdio>       // perror
#endif

int main(int argc, char* argv[]) {
    std::cout << "[main] start symulacji serwisu samochodowego" << std::endl;

    // Inicjalizacja IPC
    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[main] blad inicjalizacji IPC" << std::endl;
        return EXIT_FAILURE;
    }

#if defined(__unix__) || defined(__APPLE__)

    // ================== Kod tylko dla Unix (Linux / macOS) ==================

    // Tworzymy proces potomny dla programu "kierowca"
    pid_t pid = fork();
    if (pid < 0) {
        // fork nie powiodl sie
        perror("[main] blad fork");
        serwis_ipc_cleanup();
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Kod procesu potomnego
        const char* sciezka = "./kierowca";
        std::cout << "[main-child] uruchamiam program " << sciezka << std::endl;

        // execl zamienia biezacy proces na nowy program
        execl(sciezka, "kierowca", (char*)nullptr);

        // Jezeli jestesmy nadal tutaj, execl sie nie powiodl
        perror("[main-child] blad execl");
        _exit(1); // w procesie potomnym po bledzie exec uzywamy _exit
    } else {
        // Kod procesu macierzystego
        std::cout << "[main-parent] utworzono proces potomny, pid = " << pid << std::endl;

        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) {
            perror("[main-parent] blad waitpid");
            serwis_ipc_cleanup();
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            std::cout << "[main-parent] proces potomny zakonczyl sie z kodem "
                      << exit_code << std::endl;
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            std::cout << "[main-parent] proces potomny zostal zakonczony sygnalem "
                      << sig << std::endl;
        } else {
            std::cout << "[main-parent] proces potomny zakonczyl sie w nietypowym stanie"
                      << std::endl;
        }
    }

#else

    // ================== Kod dla Windows / innych systemow ==================

    std::cout << "[main] wykryto system nie-Unix (np. Windows)" << std::endl;
    std::cout << "[main] fork/exec/waitpid sa pominiete w tym srodowisku" << std::endl;
    std::cout << "[main] funkcje procesowe beda dzialac na maszynie z Linux" << std::endl;

#endif

    // Sprzatanie IPC
    serwis_ipc_cleanup();

    std::cout << "[main] koniec symulacji serwisu" << std::endl;
    return EXIT_SUCCESS;
}
