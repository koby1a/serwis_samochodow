// tests/test_model.cpp
#include <iostream>
#include <cassert>
#include "../model.h"

void test_serwis_czy_marka_obslugiwana() {
    // Marki obsługiwane (duże litery)
    assert(serwis_czy_marka_obslugiwana('A') == 1);
    assert(serwis_czy_marka_obslugiwana('E') == 1);
    assert(serwis_czy_marka_obslugiwana('I') == 1);
    assert(serwis_czy_marka_obslugiwana('O') == 1);
    assert(serwis_czy_marka_obslugiwana('U') == 1);
    assert(serwis_czy_marka_obslugiwana('Y') == 1);

    // Marki obsługiwane (małe litery – sprawdzamy, czy zostala zmieniona na wielka)
    assert(serwis_czy_marka_obslugiwana('a') == 1);
    assert(serwis_czy_marka_obslugiwana('e') == 1);
    assert(serwis_czy_marka_obslugiwana('y') == 1);

    // Marki nieobsługiwane
    assert(serwis_czy_marka_obslugiwana('B') == 0);
    assert(serwis_czy_marka_obslugiwana('Z') == 0);
    assert(serwis_czy_marka_obslugiwana('x') == 0);

    std::cout << "test_serwis_czy_marka_obslugiwana: OK\n";
}
void test_serwis_wybierz_stanowisko() {
    // Przygotowujemy 8 stanowisk
    Stanowisko st[8];
    for (int i = 0; i < 8; ++i) {
        st[i].id = i + 1;            // id 1..8
        st[i].zajete = 0;            // wszystkie wolne na start
        st[i].czy_tylko_UY = (i == 7) ? 1 : 0; // indeks 7 -> stanowisko 8
    }

    Samochod s1{'A', 0, 10, 0};
    int idx1 = serwis_wybierz_stanowisko(s1, st, 8);
    assert(idx1 == 0); // pierwsze wolne stanowisko (id=1)

    // Zajmujemy stanowisko 1
    st[0].zajete = 1;

    Samochod s2{'E', 0, 10, 0};
    int idx2 = serwis_wybierz_stanowisko(s2, st, 8);
    assert(idx2 == 1); // teraz pierwsze wolne to indeks 1 (id=2)

    // Zajmujemy stanowiska 1–7
    for (int i = 0; i < 7; ++i) {
        st[i].zajete = 1;
    }
    st[7].zajete = 0; // stanowisko 8 wolne, czy_tylko_UY = 1

    Samochod s3{'U', 0, 10, 0};
    int idx3 = serwis_wybierz_stanowisko(s3, st, 8);
    assert(idx3 == 7); // jedyne wolne to stanowisko 8, a U jest dozwolone

    Samochod s4{'A', 0, 10, 0};
    int idx4 = serwis_wybierz_stanowisko(s4, st, 8);
    assert(idx4 == -1); // stanowisko 8 nie obsłuży marki A

    std::cout << "test_serwis_wybierz_stanowisko: OK\n";
}

int main() {
    test_serwis_czy_marka_obslugiwana();
    test_serwis_wybierz_stanowisko();
    std::cout << "Wszystkie testy modelu zaliczone.\n";
    return 0;
}
