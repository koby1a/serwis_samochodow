// tests/test_model.cpp
#include <iostream>
#include <cassert>
#include "../model.h"

void test_serwis_czy_marka_obslugiwana() {
    // Marki obslugiwane (duze litery)
    assert(serwis_czy_marka_obslugiwana('A') == 1);
    assert(serwis_czy_marka_obslugiwana('E') == 1);
    assert(serwis_czy_marka_obslugiwana('I') == 1);
    assert(serwis_czy_marka_obslugiwana('O') == 1);
    assert(serwis_czy_marka_obslugiwana('U') == 1);
    assert(serwis_czy_marka_obslugiwana('Y') == 1);

    // Marki obslugiwane (male litery - sprawdzamy konwersje)
    assert(serwis_czy_marka_obslugiwana('a') == 1);
    assert(serwis_czy_marka_obslugiwana('e') == 1);
    assert(serwis_czy_marka_obslugiwana('y') == 1);

    // Marki nieobslugiwane
    assert(serwis_czy_marka_obslugiwana('B') == 0);
    assert(serwis_czy_marka_obslugiwana('Z') == 0);
    assert(serwis_czy_marka_obslugiwana('x') == 0);

    std::cout << "test_serwis_czy_marka_obslugiwana: OK" << std::endl;
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

    std::cout << "test_serwis_wybierz_stanowisko: OK" << std::endl;
}

void test_serwis_oblicz_czas_naprawy() {
    // Tryb normalny, bez dodatkowych usterek
    int t = serwis_oblicz_czas_naprawy(10, 0, SERWIS_TRYB_NORMALNY);
    assert(t == 10);

    // Tryb normalny, z dodatkowymi usterkami
    t = serwis_oblicz_czas_naprawy(10, 5, SERWIS_TRYB_NORMALNY);
    assert(t == 15);

    // Tryb przyspieszony, bez dodatkow: 10 -> 5
    t = serwis_oblicz_czas_naprawy(10, 0, SERWIS_TRYB_PRZYSPIESZONY);
    assert(t == 5);

    // Tryb przyspieszony, z dodatkami: (10 + 6) / 2 = 8
    t = serwis_oblicz_czas_naprawy(10, 6, SERWIS_TRYB_PRZYSPIESZONY);
    assert(t == 8);

    // Sprawdzenie zaokraglania w gore: (5 + 0) -> 3
    t = serwis_oblicz_czas_naprawy(5, 0, SERWIS_TRYB_PRZYSPIESZONY);
    assert(t == 3);

    // Czasy ujemne sa traktowane jako 0
    t = serwis_oblicz_czas_naprawy(-5, -3, SERWIS_TRYB_NORMALNY);
    assert(t == 0);

    std::cout << "test_serwis_oblicz_czas_naprawy: OK" << std::endl;
}
void test_serwis_klient_akceptuje() {
    // 2% odrzucen

    // Wartosc w zakresie odrzucenia
    assert(serwis_klient_akceptuje(0, 2) == 0);
    assert(serwis_klient_akceptuje(1, 2) == 0);

    // Wartosc poza zakresem odrzucenia
    assert(serwis_klient_akceptuje(2, 2) == 1);
    assert(serwis_klient_akceptuje(50, 2) == 1);

    // Walidacja danych
    assert(serwis_klient_akceptuje(-1, 2) == 0);
    assert(serwis_klient_akceptuje(10, 200) == 0);
}
void test_serwis_klient_zgadza_sie_na_rozszerzenie() {
    // 20% odmow

    // Odmowa rozszerzenia
    assert(serwis_klient_zgadza_sie_na_rozszerzenie(0, 20) == 0);
    assert(serwis_klient_zgadza_sie_na_rozszerzenie(19, 20) == 0);

    // Zgoda na rozszerzenie
    assert(serwis_klient_zgadza_sie_na_rozszerzenie(20, 20) == 1);
    assert(serwis_klient_zgadza_sie_na_rozszerzenie(80, 20) == 1);

    // Walidacja
    assert(serwis_klient_zgadza_sie_na_rozszerzenie(-5, 20) == 0);
    assert(serwis_klient_zgadza_sie_na_rozszerzenie(10, -1) == 0);
}

void test_serwis_klient_akceptuje_warunki() {
    // 2% odrzucen

    // Odrzucenie warunkow
    assert(serwis_klient_akceptuje_warunki(0, 2) == 0);
    assert(serwis_klient_akceptuje_warunki(1, 2) == 0);

    // Akceptacja warunkow
    assert(serwis_klient_akceptuje_warunki(2, 2) == 1);
    assert(serwis_klient_akceptuje_warunki(50, 2) == 1);

    // Bledne dane
    assert(serwis_klient_akceptuje_warunki(-1, 2) == 0);
    assert(serwis_klient_akceptuje_warunki(10, 200) == 0);
}



int main() {
    test_serwis_czy_marka_obslugiwana();
    test_serwis_wybierz_stanowisko();
    test_serwis_oblicz_czas_naprawy();
    test_serwis_klient_akceptuje();
    serwis_klient_zgadza_sie_na_rozszerzenie();
    test_serwis_klient_akceptuje_warunki();


    std::cout << "Wszystkie testy modelu zaliczone." << std::endl;
    return 0;
}
