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

// Struktura opisujaca pojedyncza usluge serwisowa
struct UslugaSerwisowa {
    int id;
    const char* nazwa;
    int cena;        // cena w zl
    int czas;        // czas w minutach
};

// Zwraca wskaznik na cennik
const UslugaSerwisowa* serwis_pobierz_cennik(int* liczba_uslug);

// Oblicza koszt na podstawie listy id uslug
int serwis_oblicz_koszt(const int* lista_uslug, int liczba_uslug);


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

// Symuluje decyzje klienta o akceptacji warunkow naprawy
// losowa_wartosc: liczba z zakresu 0..99
// prog_odrzucenia: np. 2 oznacza 2% odrzucen
int serwis_klient_akceptuje(int losowa_wartosc, int prog_odrzucenia);

// Symuluje decyzje klienta o rozszerzeniu zakresu naprawy
// losowa_wartosc: liczba z zakresu 0..99
// prog_odmowy: np. 20 oznacza 20% odmow
int serwis_klient_zgadza_sie_na_rozszerzenie(int losowa_wartosc, int prog_odmowy);

// Symuluje decyzje klienta o akceptacji warunkow naprawy
// losowa_wartosc: 0..99
// prog_odrzucenia: np. 2 oznacza 2% odrzucen
int serwis_klient_akceptuje_warunki(int losowa_wartosc, int prog_odrzucenia);

// Oblicza laczny czas naprawy na podstawie listy uslug
// lista_uslug  - tablica ID uslug
// liczba_uslug - ile uslug
// czas_dodatkowy - np. za rozszerzenie naprawy
// tryb - normalny lub przyspieszony
int serwis_oblicz_czas_z_uslug(const int* lista_uslug,
                               int liczba_uslug,
                               int czas_dodatkowy,
                               SerwisTrybPracy tryb);


