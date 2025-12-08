// model.h
#pragma once

// Tryb pracy stanowiska (normalny / przyspieszony o 50%)
enum SerwisTrybPracy {
    SERWIS_TRYB_NORMALNY = 0,
    SERWIS_TRYB_PRZYSPIESZONY = 1
};

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

// Oblicza czas naprawy z uwzglednieniem dodatkowych usterek oraz trybu pracy.
//  - czas_podstawowy >= 0
//  - czas_dodatkowy >= 0 (0 gdy brak rozszerzenia zakresu)
//  - tryb: normalny lub przyspieszony (czas o 50% krotszy - zaokraglenie w gore)
int serwis_oblicz_czas_naprawy(int czas_podstawowy,
                               int czas_dodatkowy,
                               SerwisTrybPracy tryb);
