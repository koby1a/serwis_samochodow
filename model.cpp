// model.cpp
#include "model.h"
#include <climits>  // INT_MAX

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
    if (liczba_uslug) {
        *liczba_uslug = sizeof(CENNIK) / sizeof(CENNIK[0]);
    }
    return CENNIK;
}

int serwis_oblicz_koszt(const int* lista_uslug, int liczba_uslug) {
    if (!lista_uslug || liczba_uslug <= 0) {
        return 0;
    }

    int suma = 0;
    int liczba_cennika = 0;
    const UslugaSerwisowa* cennik = serwis_pobierz_cennik(&liczba_cennika);

    for (int i = 0; i < liczba_uslug; ++i) {
        int id = lista_uslug[i];

        for (int j = 0; j < liczba_cennika; ++j) {
            if (cennik[j].id == id) {
                suma += cennik[j].cena;
                break;
            }
        }
    }

    return suma;
}

int serwis_czy_marka_obslugiwana(char marka) {
    // Zamiana malych liter na wielkie, zeby 'a' i 'A' byly traktowane tak samo
    if (marka >= 'a' && marka <= 'z') {
        marka = static_cast<char>(marka - 'a' + 'A');
    }

    switch (marka) {
        case 'A':
        case 'E':
        case 'I':
        case 'O':
        case 'U':
        case 'Y':
            return 1; // marka obslugiwana
        default:
            return 0; // marka nieobslugiwana
    }
}

int serwis_wybierz_stanowisko(const Samochod& s,
                              Stanowisko* stanowiska,
                              int liczba_stanowisk) {
    // Ujednolicenie marki do wielkich liter
    char marka = s.marka;
    if (marka >= 'a' && marka <= 'z') {
        marka = static_cast<char>(marka - 'a' + 'A');
    }

    // Jezeli marka w ogole nie jest obslugiwana, nie ma sensu szukac stanowiska
    if (!serwis_czy_marka_obslugiwana(marka)) {
        return -1;
    }

    // Szukamy pierwszego wolnego stanowiska, ktore moze obsluzyc dana marke
    for (int i = 0; i < liczba_stanowisk; ++i) {
        Stanowisko& st = stanowiska[i];

        if (st.zajete) {
            // stanowisko zajete -> szukamy dalej
            continue;
        }

        if (st.czy_tylko_UY) {
            // To stanowisko (np. 8) obsluguje tylko U i Y
            if (marka == 'U' || marka == 'Y') {
                return i; // indeks stanowiska w tablicy
            } else {
                continue; // to stanowisko nie obsluzy tej marki
            }
        } else {
            // Normalne stanowisko (1–7) - skoro marka jest obslugiwana, to OK
            return i;
        }
    }

    // Nie znaleziono wolnego, pasujacego stanowiska
    return -1;
}

int serwis_oblicz_czas_naprawy(int czas_podstawowy,
                               int czas_dodatkowy,
                               SerwisTrybPracy tryb) {
    // Pilnujemy, aby nie bylo ujemnych czasow
    if (czas_podstawowy < 0) czas_podstawowy = 0;
    if (czas_dodatkowy < 0) czas_dodatkowy = 0;

    long long suma = static_cast<long long>(czas_podstawowy) +
                     static_cast<long long>(czas_dodatkowy);

    // Tryb przyspieszony: czas krotszy o 50% (dzielimy przez 2, zaokraglenie w gore)
    if (tryb == SERWIS_TRYB_PRZYSPIESZONY) {
        suma = (suma + 1) / 2; // np. 5 -> 3, 16 -> 8
    }

    if (suma < 0) {
        suma = 0;
    } else if (suma > INT_MAX) {
        suma = INT_MAX;
    }

    return static_cast<int>(suma);
}
int serwis_klient_akceptuje(int losowa_wartosc, int prog_odrzucenia) {
    // Walidacja danych wejsciowych
    if (losowa_wartosc < 0 || losowa_wartosc > 99) {
        return 0;
    }

    if (prog_odrzucenia < 0 || prog_odrzucenia > 100) {
        return 0;
    }

    // Jesli losowa wartosc miesci sie w progu odrzucenia,
    // klient NIE akceptuje warunkow
    if (losowa_wartosc < prog_odrzucenia) {
        return 0;
    }

    // W przeciwnym wypadku klient akceptuje
    return 1;
}

int serwis_klient_zgadza_sie_na_rozszerzenie(int losowa_wartosc, int prog_odmowy) {
    // Walidacja danych wejsciowych
    if (losowa_wartosc < 0 || losowa_wartosc > 99) {
        return 0;
    }

    if (prog_odmowy < 0 || prog_odmowy > 100) {
        return 0;
    }

    // Jesli losowa wartosc miesci sie w progu odmowy,
    // klient NIE zgadza sie na rozszerzenie
    if (losowa_wartosc < prog_odmowy) {
        return 0;
    }

    // W przeciwnym wypadku klient zgadza sie na rozszerzenie
    return 1;
}

int serwis_klient_akceptuje_warunki(int losowa_wartosc, int prog_odrzucenia) {
    // Sprawdzenie poprawnosci danych
    if (losowa_wartosc < 0 || losowa_wartosc > 99) {
        return 0;
    }

    if (prog_odrzucenia < 0 || prog_odrzucenia > 100) {
        return 0;
    }

    // Jesli losowa wartosc miesci sie w progu odrzucenia
    // klient NIE akceptuje warunkow
    if (losowa_wartosc < prog_odrzucenia) {
        return 0;
    }

    // W przeciwnym wypadku klient akceptuje warunki
    return 1;
}

