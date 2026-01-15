// serwis_ipc.h
#pragma once

#include "model.h"

enum SerwisIpcStatus {
    SERWIS_IPC_OK = 0,
    SERWIS_IPC_ERR = -1
};

// Inicjalizacja (otwarcie kolejek + shm + sem)
int serwis_ipc_init();

// Odpiecie (bez usuwania zasobow systemowych)
void serwis_ipc_cleanup();

// Pelne sprzatanie (IPC_RMID) - wywolywac TYLKO w orchestratorze
void serwis_ipc_cleanup_all();

// -------- Kolejki komunikatow (System V) --------
int serwis_ipc_wyslij_zgloszenie(const Samochod& s);
int serwis_ipc_odbierz_zgloszenie(Samochod& s);

int serwis_ipc_wyslij_zlecenie(const Samochod& s, int id_klienta, int stanowisko_id, const OfertaNaprawy& oferta);
int serwis_ipc_odbierz_zlecenie(int stanowisko_id, Samochod& s, int& id_klienta, OfertaNaprawy& oferta);

int serwis_ipc_wyslij_raport(int id_klienta, int rzeczywisty_czas, int koszt_koncowy, int stanowisko_id, const Samochod& s);
int serwis_ipc_odbierz_raport(int& id_klienta, int& rzeczywisty_czas, int& koszt_koncowy, int& stanowisko_id, Samochod& s);

// -------- Statystyki w SHM + sem (2. mechanizm IPC + synchronizacja) --------
struct SerwisStatystyki {
    int przyjete_zgloszenia;
    int odrzucone_marka;
    int odrzucone_poza_godzinami;
    int odrzucone_oferta;
    int brak_stanowiska;
    int wyslane_zlecenia;
    int wykonane_naprawy;
    int platnosci;
};

// Zwraca kopie statystyk (bez trzymania semafora)
int serwis_stat_pobierz(SerwisStatystyki& out);

// Inkrementacje (chronione semaforem)
void serwis_stat_inc_przyjete();
void serwis_stat_inc_odrzucone_marka();
void serwis_stat_inc_odrzucone_poza_godzinami();
void serwis_stat_inc_odrzucone_oferta();
void serwis_stat_inc_brak_stanowiska();
void serwis_stat_inc_wyslane_zlecenia();
void serwis_stat_inc_wykonane_naprawy();
void serwis_stat_inc_platnosci();
