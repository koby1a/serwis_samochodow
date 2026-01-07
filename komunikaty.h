// komunikaty.h
#pragma once

#include "model.h"

// Typ komunikatu dla zgloszen od kierowcy do pracownika serwisu
const long SERWIS_MSGTYPE_ZGLOSZENIE = 1L;

// Typ komunikatu dla zlecen od pracownika serwisu do mechanika
const long SERWIS_MSGTYPE_ZLECENIE = 2L;

// Typ komunikatu dla raportow od mechanika do kasjera
const long SERWIS_MSGTYPE_RAPORT = 3L;

// Komunikat: kierowca -> pracownik_serwisu
struct MsgZgloszenie {
    long mtype;      // musi byc pierwsze pole w strukturze
    Samochod s;      // dane samochodu
};

// Komunikat: pracownik_serwisu -> mechanik
struct MsgZlecenie {
    long mtype;           // SERWIS_MSGTYPE_ZLECENIE
    Samochod s;           // dane samochodu
    int przewidywany_czas;
    int id_klienta;
};

// Komunikat: mechanik -> kasjer
struct MsgRaport {
    long mtype;           // SERWIS_MSGTYPE_RAPORT
    int id_klienta;
    int rzeczywisty_czas;
    int koszt_koncowy;
    Samochod s;           // opcjonalnie: dane auta (zeby kasjer wiedzial co to bylo)
};

// Komunikat: pracownik_serwisu -> mechanik
struct MsgZlecenie {
    long mtype;           // SERWIS_MSGTYPE_ZLECENIE
    Samochod s;           // dane samochodu
    int id_klienta;

    // Oferta naprawy (z cennika)
    int liczba_uslug;
    int uslugi_id[10];
    int koszt_szacowany;
    int czas_szacowany;
};
