// model.cpp
#include "model.h"
#include <climits>  // INT_MAX

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

