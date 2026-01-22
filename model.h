#pragma once

struct Samochod {
    char marka;
    int czas_przyjazdu;
    int czas_naprawy;
    int krytyczna;
};

enum SerwisTrybPracy {
    SERWIS_TRYB_NORMALNY = 0,
    SERWIS_TRYB_PRZYSPIESZONY = 1
};

struct UslugaSerwisowa {
    int id;
    const char* nazwa;
    int cena;
    int czas;
};

struct OfertaNaprawy {
    int liczba_uslug;
    int uslugi[8];
    int koszt;
    int czas;
};

/**
 * @brief Losuje liczbe calkowita z przedzialu [a,b].
 * @param seed Wskaznik na seed.
 * @param a Dolna granica.
 * @param b Gorna granica.
 * @return Wylosowana wartosc.
 */
int serwis_losuj_int(unsigned int* seed, int a, int b);

/**
 * @brief Sprawdza czy marka jest obslugiwana (A,E,I,O,U,Y).
 */
int serwis_czy_marka_obslugiwana(char marka);

/**
 * @brief Sprawdza czy dane stanowisko moze obsluzyc marke.
 * @param stanowisko_id 1..8
 * @param marka Marka.
 * @return 1 gdy moze, inaczej 0.
 */
int serwis_stanowisko_moze_obsluzyc(int stanowisko_id, char marka);

/**
 * @brief Sprawdza czy czas t jest w godzinach [Tp,Tk).
 */
int serwis_czy_w_godzinach(int Tp, int Tk, int t);

/**
 * @brief Oblicza ile minut do nastepnego otwarcia.
 */
int serwis_minuty_do_otwarcia(int Tp, int Tk, int t);

/**
 * @brief Sprawdza czy klient moze czekac poza godzinami.
 */
int serwis_czy_moze_czekac_poza_godzinami(int Tp, int Tk, int T1, const Samochod& s);

/**
 * @brief Aktualizuje liczbe okienek rejestracji wg K1/K2 i dlugosci kolejki.
 */
int serwis_aktualizuj_okienka(int aktywne_okienka, int dl_kolejki, int K1, int K2);

/**
 * @brief Zwraca wskaznik na cennik oraz liczbe pozycji.
 */
const UslugaSerwisowa* serwis_pobierz_cennik(int* liczba_uslug);

/**
 * @brief Szuka uslugi po id w cenniku.
 */
const UslugaSerwisowa* serwis_znajdz_usluge(int id);

/**
 * @brief Liczy koszt na podstawie listy id uslug.
 */
int serwis_oblicz_koszt(const int* lista_uslug, int liczba_uslug);

/**
 * @brief Liczy czas naprawy z listy uslug + czas dodatkowy, z uwzglednieniem trybu.
 */
int serwis_oblicz_czas_z_uslug(const int* lista_uslug, int liczba_uslug, int czas_dodatkowy, SerwisTrybPracy tryb);

/**
 * @brief Liczy czas naprawy: czas_podstawowy + czas_dodatkowy, z uwzglednieniem trybu.
 */
int serwis_oblicz_czas_naprawy(int czas_podstawowy, int czas_dodatkowy, SerwisTrybPracy tryb);

/**
 * @brief Czy klient akceptuje warunki (prog_odrzucenia w %).
 */
int serwis_klient_akceptuje_warunki(int losowa_wartosc, int prog_odrzucenia);

/**
 * @brief Czy klient zgadza sie na rozszerzenie (prog_odmowy w %).
 */
int serwis_klient_zgadza_sie_na_rozszerzenie(int losowa_wartosc, int prog_odmowy);

/**
 * @brief Tworzy oferte: losuje uslugi, liczy koszt i czas.
 */
int serwis_utworz_oferte(OfertaNaprawy* out, unsigned int* seed, int min_uslug, int max_uslug, int czas_dodatkowy, SerwisTrybPracy tryb);
