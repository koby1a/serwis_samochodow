// komunikaty.h
#pragma once

#include "model.h"

// Typ komunikatu dla zgloszen od kierowcy do pracownika serwisu
const long SERWIS_MSGTYPE_ZGLOSZENIE = 1L;

// Typ komunikatu dla zlecen od pracownika serwisu do mechanika
const long SERWIS_MSGTYPE_ZLECENIE = 2L;

// Komunikat: kierowca -> pracownik_serwisu
struct MsgZgloszenie {
    long mtype;      // musi byc pierwsze pole w strukturze
    Samochod s;      // dane samochodu
};

// Komunikat: pracownik_serwisu -> mechanik
struct MsgZlecenie {
    long mtype;           // typ komunikatu (SERWIS_MSGTYPE_ZLECENIE)
    Samochod s;           // dane samochodu
    int przewidywany_czas;
    int id_klienta;       // proste ID klienta, przydzielane przez pracownika
};
