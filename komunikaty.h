// komunikaty.h
#pragma once

#include "model.h"

// mtype dla zgloszen: kierowca -> pracownik_serwisu
const long SERWIS_MSGTYPE_ZGLOSZENIE = 1L;

// mtype dla raportow: mechanik -> kasjer
const long SERWIS_MSGTYPE_RAPORT = 3L;

// Baza mtype dla zlecen: mtype = SERWIS_MTYPE_ZLECENIE_BASE + stanowisko_id (1..8)
const long SERWIS_MTYPE_ZLECENIE_BASE = 100L;

// Komunikat: kierowca -> pracownik_serwisu
struct MsgZgloszenie {
    long mtype;
    Samochod s;
};

// Komunikat: pracownik_serwisu -> mechanik (routing po mtype)
struct MsgZlecenie {
    long mtype;           // SERWIS_MTYPE_ZLECENIE_BASE + stanowisko_id
    Samochod s;
    int id_klienta;
    int stanowisko_id;    // 1..8

    // Oferta naprawy
    int liczba_uslug;
    int uslugi_id[10];
    int koszt_szacowany;
    int czas_szacowany;
};

// Komunikat: mechanik -> kasjer
struct MsgRaport {
    long mtype;           // SERWIS_MSGTYPE_RAPORT
    int id_klienta;
    int rzeczywisty_czas;
    int koszt_koncowy;
    int stanowisko_id;    // 1..8
    Samochod s;
};
