#pragma once
/** @file model.h */

/** @brief Tryb pracy stanowiska/mechanika. */
enum SerwisTrybPracy {
    SERWIS_TRYB_NORMALNY = 0,
    SERWIS_TRYB_PRZYSPIESZONY = 1
};

/** @brief Dane samochodu w symulacji. */
struct Samochod {
    char marka;          // 'A'..'Z'
    int czas_przyjazdu;  // czas przyjazdu
    int czas_naprawy;    // opcjonalnie
    int krytyczna;       // 0/1
    int krytyczna_typ;   // 0=brak, 1..3 typ krytyczny
};

/** @brief Pojedyncza usluga w cenniku. */
struct UslugaSerwisowa {
    int id;
    const char* nazwa;
    int cena; // PLN
    int czas; // min
};

/** @brief Oferta naprawy przygotowana przez pracownika.
 *
 * Uwaga: model.cpp zaklada istnienie tych pol:
 * - uslugi[8]
 * - liczba_uslug
 */
struct OfertaNaprawy {
    int czas;
    int koszt;
    int uslugi[8];
    int liczba_uslug;
};

/** @brief Zlecenie wysylane do mechanika. */
struct Zlecenie {
    int id_klienta;
    int stanowisko_id; // 1..8
    Samochod s;
    OfertaNaprawy oferta;
};

/** @brief Raport po naprawie. */
struct Raport {
    int id_klienta;
    int stanowisko_id; // 1..8
    Samochod s;
    int czas;
    int koszt;
};

/** @brief Sprawdza czy marka jest obslugiwana (A,E,I,O,U,Y). */
int serwis_czy_marka_obslugiwana(char marka);

/** @brief Wybiera indeks stanowiska (0..n-1) dla samochodu albo -1 gdy brak.
 *
 * Ta funkcja jest uzywana w testach, wiec zostawiamy podpis jak masz w model.cpp.
 */
struct Stanowisko;
int serwis_wybierz_stanowisko(const Samochod& s, Stanowisko* stanowiska, int liczba_stanowisk);

/** @brief Oblicza czas naprawy (podstawowy + dodatkowy) z uwzglednieniem trybu pracy. */
int serwis_oblicz_czas_naprawy(int czas_podstawowy, int czas_dodatkowy, SerwisTrybPracy tryb);

/** @brief Zwraca wskaznik na tablice cennika i zapisuje liczbe uslug. */
const UslugaSerwisowa* serwis_pobierz_cennik(int* liczba_uslug);

/** @brief Szuka uslugi po ID w cenniku. */
const UslugaSerwisowa* serwis_znajdz_usluge(int id);

/** @brief Sumuje koszt na podstawie listy ID uslug. */
int serwis_oblicz_koszt(const int* lista_uslug, int liczba_uslug);

/** @brief Sumuje czas uslug + czas_dodatkowy i uwzglednia tryb pracy. */
int serwis_oblicz_czas_z_uslug(const int* lista_uslug,
                               int liczba_uslug,
                               int czas_dodatkowy,
                               SerwisTrybPracy tryb);

/** @brief Proste losowanie int z zakresu [a,b] (LCG). */
int serwis_losuj_int(unsigned int* seed, int a, int b);

/** @brief Czy klient akceptuje warunki (prog_odrzucenia w %). */
int serwis_klient_akceptuje_warunki(int losowa_wartosc, int prog_odrzucenia);

/** @brief Czy klient zgadza sie na rozszerzenie (prog_odmowy w %). */
int serwis_klient_zgadza_sie_na_rozszerzenie(int losowa_wartosc, int prog_odmowy);

/** @brief Tworzy oferte naprawy na bazie losowych uslug z cennika. */
int serwis_utworz_oferte(OfertaNaprawy* out, unsigned int* seed,
                         int min_uslug, int max_uslug,
                         int czas_dodatkowy, SerwisTrybPracy tryb);

/** @brief Aktualizacja liczby okienek obslugi (K1/K2). */
int serwis_aktualizuj_okienka(int aktualne, int kolejka, int K1, int K2);

/** @brief Czy samochod moze czekac poza godzinami (krytyczna lub do otwarcia < T1). */
int serwis_czy_moze_czekac_poza_godzinami(int Tp, int Tk, int T1, const Samochod& s);

/** @brief Czy stanowisko moze obsluzyc marke (8 tylko U/Y). */
int serwis_stanowisko_moze_obsluzyc(int stanowisko_id, char marka);
