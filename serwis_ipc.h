// serwis_ipc.h
#pragma once

#include "model.h"
#include "komunikaty.h"

// Proste statusy zwrotne dla funkcji IPC
enum SerwisIpcStatus {
    SERWIS_IPC_OK = 0,
    SERWIS_IPC_ERR = -1
};

// Inicjalizacja struktur IPC (pamiec dzielona, kolejki komunikatow, semafory, itp.).
// Teraz obslugujemy:
//  - kolejke zgloszen od kierowcow
//  - kolejke zlecen do mechanikow
int serwis_ipc_init();

// Sprzatanie struktur IPC na koncu dzialania programu.
// UWAGA: zakladamy, ze wykonuje to TYLKO proces glowny lub proces "odpowiedzialny".
void serwis_ipc_cleanup();

// --- Kolejka zgloszen (kierowca -> pracownik_serwisu) ---

int serwis_ipc_get_msgid_zgloszenia();
int serwis_ipc_wyslij_zgloszenie(const Samochod& s);
int serwis_ipc_odbierz_zgloszenie(Samochod& s);

// --- Kolejka zlecen (pracownik_serwisu -> mechanik) ---

// Na razie prosty interfejs: wyslanie i odebranie zlecenia.
int serwis_ipc_wyslij_zlecenie(const Samochod& s,
                               int przewidywany_czas,
                               int id_klienta);

int serwis_ipc_odbierz_zlecenie(Samochod& s,
                                int& przewidywany_czas,
                                int& id_klienta);
