#include "model.h"
#include <climits>

/** @brief Normalizuje litere marki do wielkiej. */
static char norm_marka(char m) {
    if (m >= 'a' && m <= 'z') return (char)(m - 'a' + 'A');
    return m;
}

int serwis_losuj_int(unsigned int* seed, int a, int b) {
    if (!seed) return a;
    if (a > b) { int t = a; a = b; b = t; }
    *seed = (*seed * 1103515245u + 12345u);
    unsigned int r = (*seed / 65536u) % 32768u;
    int span = (b - a + 1);
    return a + (int)(r % (unsigned int)span);
}

int serwis_czy_marka_obslugiwana(char marka) {
    marka = norm_marka(marka);
    return (marka=='A'||marka=='E'||marka=='I'||marka=='O'||marka=='U'||marka=='Y') ? 1 : 0;
}

int serwis_stanowisko_moze_obsluzyc(int stanowisko_id, char marka) {
    marka = norm_marka(marka);
    if (!serwis_czy_marka_obslugiwana(marka)) return 0;
    if (stanowisko_id == 8) return (marka=='U' || marka=='Y') ? 1 : 0;
    return (stanowisko_id >= 1 && stanowisko_id <= 7) ? 1 : 0;
}

int serwis_czy_w_godzinach(int Tp, int Tk, int t) {
    return (t >= Tp && t < Tk) ? 1 : 0;
}

int serwis_minuty_do_otwarcia(int Tp, int Tk, int t) {
    if (t < Tp) return Tp - t;
    if (t >= Tk) return (1440 - t) + Tp;
    return 0;
}

int serwis_czy_moze_czekac_poza_godzinami(int Tp, int Tk, int T1, const Samochod& s) {
    if (serwis_czy_w_godzinach(Tp, Tk, s.czas_przyjazdu)) return 1;
    if (s.krytyczna) return 1;
    int do_otwarcia = serwis_minuty_do_otwarcia(Tp, Tk, s.czas_przyjazdu);
    return (do_otwarcia < T1) ? 1 : 0;
}

int serwis_aktualizuj_okienka(int aktywne_okienka, int dl_kolejki, int K1, int K2) {
    if (aktywne_okienka < 1) aktywne_okienka = 1;
    if (aktywne_okienka > 3) aktywne_okienka = 3;

    if (dl_kolejki > K2) aktywne_okienka = 3;
    else if (dl_kolejki > K1 && aktywne_okienka < 2) aktywne_okienka = 2;

    if (aktywne_okienka >= 2 && dl_kolejki <= 2) aktywne_okienka = 1;
    if (aktywne_okienka == 3 && dl_kolejki <= 3) {
        aktywne_okienka = 2;
        if (dl_kolejki <= 2) aktywne_okienka = 1;
    }
    return aktywne_okienka;
}

static const UslugaSerwisowa CENNIK[] = {
    {1, "wymiana_oleju", 150, 30},
    {2, "wymiana_filtrow", 120, 25},
    {3, "diagnostyka_komputerowa", 100, 20},
    {4, "wymiana_klockow", 300, 60},
    {5, "wymiana_tarcz", 500, 90},
    {6, "wymiana_sprzegla", 1200, 240},
    {7, "wymiana_rozrzadu", 1400, 300},
    {8, "naprawa_zawieszenia", 800, 180},
    {9, "geometria_kol", 200, 40},
    {10, "wymiana_amortyzatorow", 900, 200},
    {11, "naprawa_ukladu_hamulcowego", 700, 150},
    {12, "wymiana_akumulatora", 180, 20},
    {13, "wymiana_swiec", 160, 30},
    {14, "czyszczenie_egr", 350, 90},
    {15, "naprawa_turbiny", 2000, 360},
    {16, "wymiana_paska", 220, 45},
    {17, "wymiana_chlodnicy", 600, 120},
    {18, "naprawa_klimatyzacji", 500, 100},
    {19, "nabicie_klimy", 250, 40},
    {20, "wymiana_oleju_skrzynia", 400, 60},
    {21, "naprawa_wydechu", 450, 90},
    {22, "wymiana_sondy_lambda", 380, 70},
    {23, "naprawa_elektryki", 300, 80},
    {24, "wymiana_pompy_paliwa", 700, 150},
    {25, "wymiana_pompy_wody", 550, 120},
    {26, "wymiana_lozyska", 480, 100},
    {27, "regeneracja_dpf", 900, 180},
    {28, "wymiana_wtryskiwaczy", 1600, 240},
    {29, "naprawa_skrzyni", 3000, 600},
    {30, "kontrola_ogolna", 100, 20}
};

const UslugaSerwisowa* serwis_pobierz_cennik(int* liczba_uslug) {
    if (liczba_uslug) *liczba_uslug = (int)(sizeof(CENNIK)/sizeof(CENNIK[0]));
    return CENNIK;
}

const UslugaSerwisowa* serwis_znajdz_usluge(int id) {
    int n = 0;
    const UslugaSerwisowa* c = serwis_pobierz_cennik(&n);
    for (int i = 0; i < n; ++i) if (c[i].id == id) return &c[i];
    return nullptr;
}

int serwis_oblicz_koszt(const int* lista_uslug, int liczba_uslug) {
    if (!lista_uslug || liczba_uslug <= 0) return 0;
    long long suma = 0;
    for (int i = 0; i < liczba_uslug; ++i) {
        const UslugaSerwisowa* u = serwis_znajdz_usluge(lista_uslug[i]);
        if (u) suma += u->cena;
    }
    if (suma < 0) suma = 0;
    if (suma > INT_MAX) suma = INT_MAX;
    return (int)suma;
}

int serwis_oblicz_czas_naprawy(int czas_podstawowy, int czas_dodatkowy, SerwisTrybPracy tryb) {
    if (czas_podstawowy < 0) czas_podstawowy = 0;
    if (czas_dodatkowy < 0) czas_dodatkowy = 0;
    long long suma = (long long)czas_podstawowy + (long long)czas_dodatkowy;
    if (tryb == SERWIS_TRYB_PRZYSPIESZONY) suma = (suma + 1) / 2;
    if (suma < 0) suma = 0;
    if (suma > INT_MAX) suma = INT_MAX;
    return (int)suma;
}

int serwis_oblicz_czas_z_uslug(const int* lista_uslug, int liczba_uslug, int czas_dodatkowy, SerwisTrybPracy tryb) {
    if (!lista_uslug || liczba_uslug <= 0) return 0;
    long long suma = 0;
    for (int i = 0; i < liczba_uslug; ++i) {
        const UslugaSerwisowa* u = serwis_znajdz_usluge(lista_uslug[i]);
        if (u) suma += u->czas;
    }
    if (czas_dodatkowy < 0) czas_dodatkowy = 0;
    suma += czas_dodatkowy;
    if (tryb == SERWIS_TRYB_PRZYSPIESZONY) suma = (suma + 1) / 2;
    if (suma < 0) suma = 0;
    if (suma > INT_MAX) suma = INT_MAX;
    return (int)suma;
}

int serwis_klient_akceptuje_warunki(int losowa_wartosc, int prog_odrzucenia) {
    if (losowa_wartosc < 0 || losowa_wartosc > 99) return 0;
    if (prog_odrzucenia < 0 || prog_odrzucenia > 100) return 0;
    return (losowa_wartosc < prog_odrzucenia) ? 0 : 1;
}

int serwis_klient_zgadza_sie_na_rozszerzenie(int losowa_wartosc, int prog_odmowy) {
    if (losowa_wartosc < 0 || losowa_wartosc > 99) return 0;
    if (prog_odmowy < 0 || prog_odmowy > 100) return 0;
    return (losowa_wartosc < prog_odmowy) ? 0 : 1;
}

int serwis_utworz_oferte(OfertaNaprawy* out, unsigned int* seed, int min_uslug, int max_uslug, int czas_dodatkowy, SerwisTrybPracy tryb) {
    if (!out || !seed) return 0;
    if (min_uslug < 1) min_uslug = 1;
    if (max_uslug > 8) max_uslug = 8;
    if (min_uslug > max_uslug) { int t = min_uslug; min_uslug = max_uslug; max_uslug = t; }

    int liczba = serwis_losuj_int(seed, min_uslug, max_uslug);
    out->liczba_uslug = liczba;

    int n = 0;
    serwis_pobierz_cennik(&n);

    for (int i = 0; i < liczba; ++i) out->uslugi[i] = serwis_losuj_int(seed, 1, n);
    for (int i = liczba; i < 8; ++i) out->uslugi[i] = 0;

    out->koszt = serwis_oblicz_koszt(out->uslugi, out->liczba_uslug);
    out->czas  = serwis_oblicz_czas_z_uslug(out->uslugi, out->liczba_uslug, czas_dodatkowy, tryb);
    return 1;
}
