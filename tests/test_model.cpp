// tests/test_model.cpp
#include <iostream>
#include <cassert>
#include "../model.h"

void test_serwis_czy_marka_obslugiwana() {
    // Marki obslugiowane (duze litery)
    assert(serwis_czy_marka_obslugiwana('A') == 1);
    assert(serwis_czy_marka_obslugiwana('E') == 1);
    assert(serwis_czy_marka_obslugiwana('I') == 1);
    assert(serwis_czy_marka_obslugiwana('O') == 1);
    assert(serwis_czy_marka_obslugiwana('U') == 1);
    assert(serwis_czy_marka_obslugiwana('Y') == 1);

    // Marki obslugiowane (male litery - sprawdzamy konwersje)
    assert(serwis_czy_marka_obslugiwana('a') == 1);
    assert(serwis_czy_marka_obslugiwana('e') == 1);
    assert(serwis_czy_marka_obslugiwana('y') == 1);

    // Marki nieobslugiowane
    assert(serwis_czy_marka_obslugiwana('B') == 0);
    assert(serwis_czy_marka_obslugiwana('Z') == 0);
    assert(serwis_czy_marka_obslugiwana('x') == 0);

    std::cout << "test_serwis_czy_marka_obslugiwana: OK\n";
}

void test_serwis_wybierz_stanowisko() {
    Stanowisko stanowiska[2];

    // Stanowisko 0 - normalne (1–7), wolne
    stanowiska[0].id = 1;
    stanowiska[0].czy_tylko_UY = 0;
    stanowiska[0].zajete = 0;

    // Stanowisko 1 - tylko U/Y (8), wolne
    stanowiska[1].id = 8;
    stanowiska[1].czy_tylko_UY = 1;
    stanowiska[1].zajete = 0;

    Samochod s{};

    // 1) Marka A -> normalne stanowisko 0
    s.marka = 'A';
    int idx = serwis_wybierz_stanowisko(s, stanowiska, 2);
    assert(idx == 0);

    // 2) Marka U, oba wolne -> wybieramy pierwsze (0)
    s.marka = 'U';
    idx = serwis_wybierz_stanowisko(s, stanowiska, 2);
    assert(idx == 0);

    // 3) Zajmujemy 0, marka U -> powinnismy dostac 1 (stanowisko U/Y)
    stanowiska[0].zajete = 1;
    s.marka = 'U';
    idx = serwis_wybierz_stanowisko(s, stanowiska, 2);
    assert(idx == 1);

    // 4) Tylko stanowisko 1 wolne, marka A -> brak miejsca (-1)
    s.marka = 'A';
    idx = serwis_wybierz_stanowisko(s, stanowiska, 2);
    assert(idx == -1);

    // 5) Marka w ogole nieobslugiwana (Z) -> od razu -1
    s.marka = 'Z';
    stanowiska[0].zajete = 0;
    stanowiska[1].zajete = 0;
    idx = serwis_wybierz_stanowisko(s, stanowiska, 2);
    assert(idx == -1);

    std::cout << "test_serwis_wybierz_stanowisko: OK\n";
}

int main() {
    test_serwis_czy_marka_obslugiwana();
    test_serwis_wybierz_stanowisko();

    std::cout << "Wszystkie testy modelu zaliczone.\n";
    return 0;
}
