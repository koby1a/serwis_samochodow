// serwis_ipc.h
#pragma once

#include "model.h"
#include "komunikaty.h"

const int SERWIS_IPC_OK = 0;
const int SERWIS_IPC_ERR = -1;

// Inicjalizacja i sprzatanie IPC (na Windows stub)
int serwis_ipc_init();
void serwis_ipc_cleanup();

// --- Kolejka zgloszen (kierowca -> pracownik_serwisu) ---
int serwis_ipc_wyslij_zgloszenie(const Samochod& s);
int serwis_ipc_odbierz_zgloszenie(Samochod& s);

// --- Kolejka zlecen (pracownik_serwisu -> mechanik) ---
// Routing po mtype: SERWIS_MTYPE_ZLECENIE_BASE + stanowisko_id
int serwis_ipc_wyslij_zlecenie(const Samochod& s,
                               int id_klienta,
                               int stanowisko_id,
                               const OfertaNaprawy& oferta);

int serwis_ipc_odbierz_zlecenie(int stanowisko_id,
                                Samochod& s,
                                int& id_klienta,
                                OfertaNaprawy& oferta);

// --- Kolejka raportow (mechanik -> kasjer) ---
int serwis_ipc_wyslij_raport(int id_klienta,
                             int rzeczywisty_czas,
                             int koszt_koncowy,
                             int stanowisko_id,
                             const Samochod& s);

int serwis_ipc_odbierz_raport(int& id_klienta,
                              int& rzeczywisty_czas,
                              int& koszt_koncowy,
                              int& stanowisko_id,
                              Samochod& s);
