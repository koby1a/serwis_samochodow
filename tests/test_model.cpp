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
void test_serwis_cennik() {
    int liczba = 0;
    const UslugaSerwisowa* cennik = serwis_pobierz_cennik(&liczba);

    assert(cennik != nullptr);
    assert(liczba >= 30);
}

void test_serwis_oblicz_koszt() {
    int uslugi1[] = {1};
    assert(serwis_oblicz_koszt(uslugi1, 1) == 150);

    int uslugi2[] = {1, 2, 3};
    assert(serwis_oblicz_koszt(uslugi2, 3) == 370);

    int uslugi3[] = {999};
    assert(serwis_oblicz_koszt(uslugi3, 1) == 0);

    assert(serwis_oblicz_koszt(nullptr, 2) == 0);
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


void test_serwis_oblicz_czas_z_uslug() {
    // Jedna usluga
    int uslugi1[] = {1}; // wymiana_oleju -> 30
    assert(serwis_oblicz_czas_z_uslug(uslugi1, 1, 0, SERWIS_TRYB_NORMALNY) == 30);

    // Kilka uslug
    int uslugi2[] = {1, 2, 3}; // 30 + 25 + 20 = 75
    assert(serwis_oblicz_czas_z_uslug(uslugi2, 3, 0, SERWIS_TRYB_NORMALNY) == 75);

    // Czas dodatkowy
    assert(serwis_oblicz_czas_z_uslug(uslugi2, 3, 10, SERWIS_TRYB_NORMALNY) == 85);

    // Tryb przyspieszony (50%)
    assert(serwis_oblicz_czas_z_uslug(uslugi2, 3, 0, SERWIS_TRYB_PRZYSPIESZONY) == 38);

    // Parzyste 80 -> 40
    assert(serwis_oblicz_czas_z_uslug(uslugi2, 3, 5, SERWIS_TRYB_PRZYSPIESZONY) == 40); // 75+5=80 -> 40


    // Bledne dane
    assert(serwis_oblicz_czas_z_uslug(nullptr, 3, 0, SERWIS_TRYB_NORMALNY) == 0);
    assert(serwis_oblicz_czas_z_uslug(uslugi1, 0, 0, SERWIS_TRYB_NORMALNY) == 0);
}

void test_serwis_znajdz_usluge() {
    const UslugaSerwisowa* u = serwis_znajdz_usluge(1);
    assert(u != nullptr);
    assert(u->id == 1);
    assert(u->cena == 150);
    assert(u->czas == 30);

    // Nieistniejace ID
    assert(serwis_znajdz_usluge(999) == nullptr);
}

void test_serwis_losuj_liste_uslug() {
    unsigned int seed = 12345u;

    int uslugi[10];
    int n = serwis_losuj_liste_uslug(uslugi, 10, &seed, 2, 5);

    // Liczba uslug w zakresie
    assert(n >= 2 && n <= 5);

    // ID poprawne i bez duplikatow
    for (int i = 0; i < n; ++i) {
        assert(uslugi[i] >= 1 && uslugi[i] <= 30);
        for (int j = i + 1; j < n; ++j) {
            assert(uslugi[i] != uslugi[j]);
        }
    }

    // Determinizm: przy tym samym seed wynik powinien byc taki sam
    unsigned int seed2 = 12345u;
    int uslugi2[10];
    int n2 = serwis_losuj_liste_uslug(uslugi2, 10, &seed2, 2, 5);

    assert(n2 == n);
    for (int i = 0; i < n; ++i) {
        assert(uslugi2[i] == uslugi[i]);
    }
}


void test_serwis_utworz_oferte() {
    unsigned int seed = 777u;

    OfertaNaprawy o{};
    int ok = serwis_utworz_oferte(&o, &seed, 2, 5, 0, SERWIS_TRYB_NORMALNY);

    assert(ok == 1);
    assert(o.liczba_uslug >= 2 && o.liczba_uslug <= 5);
    assert(o.koszt > 0);
    assert(o.czas > 0);

    // Bez duplikatow i ID w zakresie
    for (int i = 0; i < o.liczba_uslug; ++i) {
        assert(o.uslugi_id[i] >= 1 && o.uslugi_id[i] <= 30);
        for (int j = i + 1; j < o.liczba_uslug; ++j) {
            assert(o.uslugi_id[i] != o.uslugi_id[j]);
        }
    }

    // Determinizm: to samo seed -> ta sama oferta
    unsigned int seed2 = 777u;
    OfertaNaprawy o2{};
    int ok2 = serwis_utworz_oferte(&o2, &seed2, 2, 5, 0, SERWIS_TRYB_NORMALNY);

    assert(ok2 == 1);
    assert(o2.liczba_uslug == o.liczba_uslug);
    assert(o2.koszt == o.koszt);
    assert(o2.czas == o.czas);
    for (int i = 0; i < o.liczba_uslug; ++i) {
        assert(o2.uslugi_id[i] == o.uslugi_id[i]);
    }
}




int main() {
    test_serwis_czy_marka_obslugiwana();
    test_serwis_cennik();
    test_serwis_wybierz_stanowisko();
    test_serwis_oblicz_czas_naprawy();
    test_serwis_klient_akceptuje();
    test_serwis_klient_zgadza_sie_na_rozszerzenie();
    test_serwis_klient_akceptuje_warunki();
    test_serwis_oblicz_czas_z_uslug();
    test_serwis_znajdz_usluge();
    test_serwis_losuj_liste_uslug();
    test_serwis_utworz_oferte();




    std::cout << "Wszystkie testy modelu zaliczone." << std::endl;
    return 0;
}



