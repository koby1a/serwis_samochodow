#pragma once
/** @file serwis_ipc.h */

#include "model.h"

#define SERWIS_IPC_OK     0
#define SERWIS_IPC_ERR    1
#define SERWIS_IPC_NO_MSG 2

/** @brief Widok stanowiska do dashboardu (SHM). */
struct SerwisStationView {
    int zajete;      // 0/1
    int zamkniete;   // 0/1
    int obsluzone;   // licznik
    char marka;      // aktualna marka
    int krytyczna;   // 0/1
    int dodatkowe;   // 0/1
    int tryb;        // SerwisTrybPracy
    int pid;         // PID mechanika
};

/** @brief Stan serwisu w SHM. Indeksy 1..8 sa uzywane. */
struct SerwisStatystyki {
    int pozar; // 0/1
    int sim_time_min; // 0..1439, czas symulacji
    SerwisStationView st[9];
    int req_close[9]; // prosba o zamkniecie stanowiska (1..8)
};

/** @brief Zapytanie o dodatkowe usterki. */
struct SerwisExtraReq {
    int id_klienta;
    int stanowisko_id;
    int czas_dod;
    int koszt_dod;
};

/** @brief Odpowiedz klienta na dodatkowe usterki. */
struct SerwisExtraResp {
    int id_klienta;
    int akceptacja; // 0/1
};

/** @brief Inicjalizuje IPC (kolejki + shm + sem). */
int serwis_ipc_init();

/** @brief Odlacza SHM w biezacym procesie. */
void serwis_ipc_detach();

/** @brief Usuwa zasoby IPC (kolejki/shm/sem). */
void serwis_ipc_cleanup_all();

/** @brief Wysyla zgloszenie samochodu do pracownika. */
int serwis_ipc_send_zgl(const Samochod& s);

/** @brief Odbiera zgloszenie samochodu (blokujaco, odporne na EINTR). */
int serwis_ipc_recv_zgl(Samochod& s);

/** @brief Odbiera zgloszenie nieblokujaco (NO_MSG gdy brak). */
int serwis_ipc_try_recv_zgl(Samochod& s);

/** @brief Wysyla zlecenie do mechanika (mtype = 100 + stanowisko_id). */
int serwis_ipc_send_zlec(const Zlecenie& z);

/** @brief Odbiera zlecenie dla stanowiska (blokujaco, odporne na EINTR). */
int serwis_ipc_recv_zlec(int stanowisko_id, Zlecenie& z);

/** @brief Wysyla raport do pracownika. */
int serwis_ipc_send_rap(const Raport& r);

/** @brief Odbiera raport (blokujaco, odporne na EINTR). */
int serwis_ipc_recv_rap(Raport& r);

/** @brief Odbiera raport nieblokujaco (NO_MSG gdy brak). */
int serwis_ipc_try_recv_rap(Raport& r);

/** @brief Wysyla raport do kasy. */
int serwis_ipc_send_kasa(const Raport& r);

/** @brief Odbiera raport w kasie (blokujaco). */
int serwis_ipc_recv_kasa(Raport& r);

/** @brief Wysyla zapytanie o dodatkowe usterki. */
int serwis_ipc_send_extra_req(const SerwisExtraReq& r);

/** @brief Odbiera zapytanie o dodatkowe usterki (blokujaco). */
int serwis_ipc_recv_extra_req(SerwisExtraReq& r);

/** @brief Odbiera zapytanie o dodatkowe usterki (nieblokujaco). */
int serwis_ipc_try_recv_extra_req(SerwisExtraReq& r);

/** @brief Wysyla odpowiedz na dodatkowe usterki. */
int serwis_ipc_send_extra_resp(const SerwisExtraResp& r);

/** @brief Odbiera odpowiedz dla klienta (blokujaco). */
int serwis_ipc_recv_extra_resp(int id_klienta, SerwisExtraResp& r);

/** @brief Pobiera kopie statystyk z SHM. */
int serwis_stat_get(SerwisStatystyki& out);

/** @brief Ustawia flage pozaru. */
void serwis_set_pozar(int v);

/** @brief Pobiera flage pozaru. */
int serwis_get_pozar();

/** @brief Ustawia czas symulacji (minuty 0..1439). */
void serwis_time_set(int minuty);

/** @brief Pobiera czas symulacji (minuty 0..1439). */
int serwis_time_get();

/** @brief Aktualizuje widok stanowiska. */
void serwis_station_set_busy(int id, int busy, char marka, int kryt, int dodatkowe, int tryb);

/** @brief Zwieksza licznik obsluzonych dla stanowiska. */
void serwis_station_inc_done(int id);

/** @brief Ustawia czy stanowisko jest zamkniete. */
void serwis_station_set_closed(int id, int closed);

/** @brief Ustawia prosbe o zamkniecie stanowiska. */
void serwis_req_close(int id, int v);

/** @brief Pobiera prosbe o zamkniecie stanowiska. */
int serwis_get_req_close(int id);

/** @brief Ustawia PID mechanika dla stanowiska. */
void serwis_station_set_pid(int id, int pid);

/** @brief Pobiera PID mechanika dla stanowiska. */
int serwis_station_get_pid(int id);
