// src/model.cpp
#include "model.h"

int serwis_czy_marka_obslugiwana(char marka) {

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
            return 1; //gdy marka jest obslugiwana
        default:
            return 0; // gdy marka nie jest obslugiwana
    }
}

int serwis_wybierz_stanowisko(const Samochod &s, Stanowisko *stanowisko, int liczba_stanowisk) {
    char marka = s.marka;
    if (marka >= 'a' && marka <= 'z') {
        marka = static_cast<char>(marka - 'a' + 'A');
    }

    for (int i = 0; i < liczba_stanowisk; ++i) {
        Stanowisko& st = stanowisko[i];

        if (st.zajete) {
            continue; // stanowisko zajęte => przejdź dalej
        }

        // Jeżeli to stanowisko "czy_tylko_UY", to obsłuży tylko U lub Y
        if (st.czy_tylko_UY) {
            if (marka == 'U' || marka == 'Y') {
                return i; //indeks tablicy
            } else {
                continue; // to stanowisko nie obsłuży tej marki
            }
        } else {
            // Normalne stanowisko 1–7, obsługuje wszystkie dozwolone marki
            // (A, E, I, O, U, Y)
            return i;
        }
    }

    // Brak wolnego stanowiska dla tego samochodu
    return -1;
}
