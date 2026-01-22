#include <iostream>
#include <cstdlib>
#include "../model.h"

#define ASSERT_TRUE(x) do{ if(!(x)){ std::cerr<<"Assertion failed: "<<#x<<" line "<<__LINE__<<"\n"; std::exit(3);} }while(0)

/** @brief Test marek obslugiwanych. */
static void test_marki() {
    ASSERT_TRUE(serwis_czy_marka_obslugiwana('A') == 1);
    ASSERT_TRUE(serwis_czy_marka_obslugiwana('B') == 0);
    ASSERT_TRUE(serwis_czy_marka_obslugiwana('y') == 1);
}

/** @brief Test Tp/Tk/T1. */
static void test_godziny_T1() {
    Samochod s{};
    s.czas_przyjazdu = 7*60;
    s.krytyczna = 0;
    ASSERT_TRUE(serwis_czy_moze_czekac_poza_godzinami(8*60,16*60,60,s) == 0);
    ASSERT_TRUE(serwis_czy_moze_czekac_poza_godzinami(8*60,16*60,61,s) == 1);
}

/** @brief Test logiki K1/K2. */
static void test_okienka_K1K2() {
    int k = 1;
    k = serwis_aktualizuj_okienka(k, 4, 3, 5);
    ASSERT_TRUE(k == 2);
    k = serwis_aktualizuj_okienka(k, 6, 3, 5);
    ASSERT_TRUE(k == 3);
    k = serwis_aktualizuj_okienka(k, 3, 3, 5);
    ASSERT_TRUE(k == 2);
    k = serwis_aktualizuj_okienka(k, 2, 3, 5);
    ASSERT_TRUE(k == 1);
}

/** @brief Test czasu w trybie przyspieszonym. */
static void test_czas_przyspieszenie() {
    ASSERT_TRUE(serwis_oblicz_czas_naprawy(10, 0, SERWIS_TRYB_NORMALNY) == 10);
    ASSERT_TRUE(serwis_oblicz_czas_naprawy(10, 0, SERWIS_TRYB_PRZYSPIESZONY) == 5);
    ASSERT_TRUE(serwis_oblicz_czas_naprawy(11, 0, SERWIS_TRYB_PRZYSPIESZONY) == 6);
}

/** @brief Test cennika i kosztu. */
static void test_cennik() {
    int n=0;
    auto c = serwis_pobierz_cennik(&n);
    ASSERT_TRUE(c != nullptr);
    ASSERT_TRUE(n >= 30);
    int usl[3] = {1,2,3};
    int koszt = serwis_oblicz_koszt(usl,3);
    ASSERT_TRUE(koszt > 0);
}

int main() {
    test_marki(); std::cout << "test_marki: OK\n";
    test_godziny_T1(); std::cout << "test_godziny_T1: OK\n";
    test_okienka_K1K2(); std::cout << "test_okienka_K1K2: OK\n";
    test_czas_przyspieszenie(); std::cout << "test_czas_przyspieszenie: OK\n";
    test_cennik(); std::cout << "test_cennik: OK\n";
    std::cout << "Wszystkie testy jednostkowe zaliczone.\n";
    return 0;
}
