// ipc.h
#pragma once

// Proste statusy zwrotne dla funkcji IPC
enum SerwisIpcStatus {
    SERWIS_IPC_OK = 0,
    SERWIS_IPC_ERR = -1
};

// Inicjalizacja struktur IPC (pamiec dzielona, kolejki komunikatow, semafory, itp.).
// Na razie tylko szkic - wlasciwa implementacja bedzie dodana pozniej.
int serwis_ipc_init();

// Sprzatanie struktur IPC na koncu dzialania programu.
void serwis_ipc_cleanup();
