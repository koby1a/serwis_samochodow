// stanowiska_status.h
#pragma once

// Pliki flagowe (dziala na Windows i Linux):
// - stanowisko_<id>_close.req   -> prosba o zamkniecie (kierownik)
// - stanowisko_<id>_closed.flag -> stanowisko zamkniete (mechanik po zakonczeniu)
// - serwis_pozar.flag           -> pozar (kierownik)

// Zwraca 1 jesli jest plik "zamkniete"
int serwis_stanowisko_jest_zamkniete(int stanowisko_id);

// Zwraca 1 jesli jest prosba o zamkniecie
int serwis_stanowisko_ma_prosbe_zamkniecia(int stanowisko_id);

// Ustawia plik prosby o zamkniecie
int serwis_stanowisko_ustaw_prosbe_zamkniecia(int stanowisko_id);

// Usuwa plik prosby o zamkniecie
int serwis_stanowisko_usun_prosbe_zamkniecia(int stanowisko_id);

// Ustawia plik "zamkniete"
int serwis_stanowisko_ustaw_zamkniete(int stanowisko_id);

// Usuwa plik "zamkniete" (np. przy starcie mechanika)
int serwis_stanowisko_usun_zamkniete(int stanowisko_id);

// Pozar (globalny)
int serwis_pozar_jest();
int serwis_pozar_ustaw();
int serwis_pozar_usun();
