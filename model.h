// src/model.h
#pragma once

struct Samochod {
    char marka;          // 'A'..'Z'
    int czas_przyjazdu;
    int czas_naprawy;
    int krytyczna;       // 0/1
};

struct Stanowisko {
    int id;              // 1..8
    int czy_tylko_UY;    // 1 dla stanowiska 8
    int zajete;          // 0/1
    Samochod aktualny;
};

int serwis_czy_marka_obslugiwana(char marka);

int serwis_wybierz_stanowisko(const Samochod& s, Stanowisko* stanowisko, int liczba_stanowisk);