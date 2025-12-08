// model.h
#pragma once

struct Samochod {
    char marka;          // 'A'..'Z'
    int czas_przyjazdu;  // chwila przyjazdu (w jednostkach symulacji)
    int czas_naprawy;    // szacowany czas naprawy
    int krytyczna;       // 0/1 - czy usterka krytyczna
};

struct Stanowisko {
    int id;              // 1..8
    int czy_tylko_UY;    // 1 dla stanowiska 8 (tylko U/Y), 0 dla 1–7
    int zajete;          // 0 - wolne, 1 - zajete
    Samochod aktualny;   // aktualnie obslugiwany samochod
};

// Zwraca 1, jesli marka jest obslugiwana (A, E, I, O, U, Y), w przeciwnym razie 0.
int serwis_czy_marka_obslugiwana(char marka);

// Zwraca indeks stanowiska (0..liczba_stanowisk-1), ktore moze przyjac samochod,
// lub -1, jesli zadne wolne stanowisko nie moze go obsluzyc.
int serwis_wybierz_stanowisko(const Samochod& s,
                              Stanowisko* stanowiska,
                              int liczba_stanowisk);
