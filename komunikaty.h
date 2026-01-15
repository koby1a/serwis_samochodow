// komunikaty.h
#pragma once

#include "model.h"

// Typy mtype dla kolejek:
// - zgloszenia: mtype = 1
// - zlecenia:  mtype = 100 + stanowisko_id
// - raporty:   mtype = 1

struct MsgZgloszenie {
    long mtype;      // 1
    Samochod s;
};

struct MsgZlecenie {
    long mtype;      // 100 + stanowisko_id
    int stanowisko_id;
    int id_klienta;
    Samochod s;
    OfertaNaprawy oferta;
};

struct MsgRaport {
    long mtype;      // 1
    int id_klienta;
    int stanowisko_id;
    Samochod s;
    int rzeczywisty_czas;
    int koszt_koncowy;
};
