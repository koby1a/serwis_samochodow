// stanowiska_status.cpp
#include "stanowiska_status.h"
#include <string>
#include <fstream>
#include <cstdio> // std::remove

static std::string serwis_nazwa_close_req(int id) {
    return "stanowisko_" + std::to_string(id) + "_close.req";
}

static std::string serwis_nazwa_closed_flag(int id) {
    return "stanowisko_" + std::to_string(id) + "_closed.flag";
}

static int serwis_plik_istnieje(const std::string& path) {
    std::ifstream f(path.c_str());
    return f.good() ? 1 : 0;
}

static int serwis_utworz_plik(const std::string& path) {
    std::ofstream f(path.c_str(), std::ios::trunc);
    if (!f.is_open()) return 0;
    f << "1\n";
    return 1;
}

static int serwis_usun_plik(const std::string& path) {
    // remove zwraca 0 przy sukcesie
    int r = std::remove(path.c_str());
    return (r == 0) ? 1 : 0;
}

int serwis_stanowisko_jest_zamkniete(int stanowisko_id) {
    if (stanowisko_id < 1 || stanowisko_id > 8) return 1;
    return serwis_plik_istnieje(serwis_nazwa_closed_flag(stanowisko_id));
}

int serwis_stanowisko_ma_prosbe_zamkniecia(int stanowisko_id) {
    if (stanowisko_id < 1 || stanowisko_id > 8) return 1;
    return serwis_plik_istnieje(serwis_nazwa_close_req(stanowisko_id));
}

int serwis_stanowisko_ustaw_prosbe_zamkniecia(int stanowisko_id) {
    if (stanowisko_id < 1 || stanowisko_id > 8) return 0;
    return serwis_utworz_plik(serwis_nazwa_close_req(stanowisko_id));
}

int serwis_stanowisko_usun_prosbe_zamkniecia(int stanowisko_id) {
    if (stanowisko_id < 1 || stanowisko_id > 8) return 0;
    // Jesli nie istnieje, to i tak OK
    if (!serwis_stanowisko_ma_prosbe_zamkniecia(stanowisko_id)) return 1;
    return serwis_usun_plik(serwis_nazwa_close_req(stanowisko_id));
}

int serwis_stanowisko_ustaw_zamkniete(int stanowisko_id) {
    if (stanowisko_id < 1 || stanowisko_id > 8) return 0;
    return serwis_utworz_plik(serwis_nazwa_closed_flag(stanowisko_id));
}

int serwis_stanowisko_usun_zamkniete(int stanowisko_id) {
    if (stanowisko_id < 1 || stanowisko_id > 8) return 0;
    if (!serwis_stanowisko_jest_zamkniete(stanowisko_id)) return 1;
    return serwis_usun_plik(serwis_nazwa_closed_flag(stanowisko_id));
}

// -------- Pozar --------

static const char* SERWIS_POZAR_FLAG = "serwis_pozar.flag";

int serwis_pozar_jest() {
    return serwis_plik_istnieje(SERWIS_POZAR_FLAG);
}

int serwis_pozar_ustaw() {
    return serwis_utworz_plik(SERWIS_POZAR_FLAG);
}

int serwis_pozar_usun() {
    if (!serwis_pozar_jest()) return 1;
    return serwis_usun_plik(SERWIS_POZAR_FLAG);
}
