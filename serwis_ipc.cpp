// ipc.cpp
#include <iostream>
#include "serwis_ipc.h"

int serwis_ipc_init() {
    std::cout << "[IPC] inicjalizacja struktur IPC (szkic)" << std::endl;

    // TODO:
    //  - ftok()
    //  - shmget(), shmat()
    //  - msgget()
    //  - semget()
    //  - obsluga bledow (perror, errno)

    return SERWIS_IPC_OK;
}

void serwis_ipc_cleanup() {
    std::cout << "[IPC] sprzatanie struktur IPC (szkic)" << std::endl;

    // TODO:
    //  - shmctl(..., IPC_RMID)
    //  - msgctl(..., IPC_RMID)
    //  - semctl(..., IPC_RMID)
}
