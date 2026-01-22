#pragma once
#include "model.h"

enum SerwisIpcStatus { SERWIS_IPC_OK = 0, SERWIS_IPC_ERR = -1 };

struct SerwisStationView {
    int obsluzone;
    int zajete;
    char marka;
    int krytyczna;
    int dodatkowe;
    int tryb;
    int zamkniete;
};

struct SerwisStatystyki {
    int przyjete_zgloszenia;
    int odrzucone_marka;
    int odrzucone_poza_godzinami;
    int odrzucone_oferta;
    int brak_stanowiska;
    int wyslane_zlecenia;
    int wykonane_naprawy;
    int platnosci;
    int pozar;
    SerwisStationView st[9];
    int req_close[9];
};

/**
 * @brief Inicjalizuje kolejki, shm i semafor.
 */
int serwis_ipc_init();

/**
 * @brief Odlacza pamiec dzielona (shmdt).
 */
void serwis_ipc_detach();

/**
 * @brief Usuwa wszystkie zasoby IPC (IPC_RMID).
 */
void serwis_ipc_cleanup_all();

struct Zlecenie {
    int id_klienta;
    int stanowisko_id;
    Samochod s;
    OfertaNaprawy oferta;
};

struct Raport {
    int id_klienta;
    int stanowisko_id;
    Samochod s;
    int czas;
    int koszt;
};

/**
 * @brief Wysyla zgloszenie do kolejki.
 */
int serwis_ipc_send_zgl(const Samochod& s);

/**
 * @brief Odbiera zgloszenie z kolejki.
 */
int serwis_ipc_recv_zgl(Samochod& s);

/**
 * @brief Wysyla zlecenie (mtype = 100 + stanowisko_id).
 */
int serwis_ipc_send_zlec(const Zlecenie& z);

/**
 * @brief Odbiera zlecenie dla stanowiska.
 */
int serwis_ipc_recv_zlec(int stanowisko_id, Zlecenie& z);

/**
 * @brief Wysyla raport do kolejki.
 */
int serwis_ipc_send_rap(const Raport& r);

/**
 * @brief Odbiera raport z kolejki.
 */
int serwis_ipc_recv_rap(Raport& r);

/**
 * @brief Kopiuje statystyki z SHM do out.
 */
int serwis_stat_get(SerwisStatystyki& out);

/**
 * @brief Ustawia flage pozaru.
 */
void serwis_set_pozar(int v);

/**
 * @brief Pobiera flage pozaru.
 */
int serwis_get_pozar();

/**
 * @brief Aktualizuje widok stanowiska: zajete/marka/krytyczna/dodatkowe/tryb.
 */
void serwis_station_set_busy(int id, int busy, char marka, int kryt, int dodatkowe, int tryb);

/**
 * @brief Inkrementuje licznik obsluzonych aut na stanowisku.
 */
void serwis_station_inc_done(int id);

/**
 * @brief Ustawia flage zamkniecia stanowiska.
 */
void serwis_station_set_closed(int id, int closed);

/**
 * @brief Ustawia prosbe zamkniecia (req_close) dla stanowiska.
 */
void serwis_req_close(int id, int v);

/**
 * @brief Pobiera prosbe zamkniecia dla stanowiska.
 */
int serwis_get_req_close(int id);
