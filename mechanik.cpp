// mechanik.cpp
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include "model.h"
#include "serwis_ipc.h"

#if defined(__unix__) || defined(__APPLE__)
    #include <csignal>
    #include <unistd.h>
#endif

static SerwisTrybPracy g_tryb_pracy = SERWIS_TRYB_NORMALNY;
static bool g_stanowisko_otwarte = true;
static bool g_zamknij_po_biezacej = false;
static bool g_pozar = false;

static unsigned int g_seed_mechanik = 98765u;

#if defined(__unix__) || defined(__APPLE__)

void mechanik_signal_handler(int sig) {
    switch (sig) {
        case SIGUSR1:
            std::cout << "[mechanik] sygnal1: zamkniecie po biezacej naprawie" << std::endl;
            g_zamknij_po_biezacej = true;
            break;
        case SIGUSR2:
            if (g_tryb_pracy == SERWIS_TRYB_NORMALNY) {
                std::cout << "[mechanik] sygnal2: przyspieszenie o 50%" << std::endl;
                g_tryb_pracy = SERWIS_TRYB_PRZYSPIESZONY;
            } else {
                std::cout << "[mechanik] sygnal2 zignorowany (juz przyspieszony)" << std::endl;
            }
            break;
        case SIGTERM:
            if (g_tryb_pracy == SERWIS_TRYB_PRZYSPIESZONY) {
                std::cout << "[mechanik] sygnal3: powrot do normalnego" << std::endl;
                g_tryb_pracy = SERWIS_TRYB_NORMALNY;
            } else {
                std::cout << "[mechanik] sygnal3 zignorowany (juz normalny)" << std::endl;
            }
            break;
        case SIGINT:
            std::cout << "[mechanik] sygnal4: POZAR - natychmiast stop" << std::endl;
            g_pozar = true;
            break;
        default:
            std::cout << "[mechanik] nieznany sygnal: " << sig << std::endl;
            break;
    }
}

static void mechanik_zarejestruj_sygnaly() {
    struct sigaction sa{};
    sa.sa_handler = mechanik_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGUSR1");
    if (sigaction(SIGUSR2, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGUSR2");
    if (sigaction(SIGTERM, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGTERM");
    if (sigaction(SIGINT, &sa, nullptr) == -1) perror("[mechanik] sigaction SIGINT");
}

static void mechanik_zapisz_pid(int stanowisko_id) {
    std::string nazwa = "mechanik_" + std::to_string(stanowisko_id) + "_pid.txt";
    std::ofstream f(nazwa.c_str(), std::ios::trunc);
    if (f.is_open()) {
        f << getpid() << std::endl;
        std::cout << "[mechanik] zapisano PID do pliku " << nazwa << std::endl;
    } else {
        std::cerr << "[mechanik] nie moge zapisac pliku PID" << std::endl;
    }
}

#endif

int main(int argc, char** argv) {
    int stanowisko_id = 1;

    // Parsowanie: --id N
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--id") {
            stanowisko_id = std::atoi(argv[i + 1]);
        }
    }

    if (stanowisko_id < 1 || stanowisko_id > 8) {
        std::cerr << "[mechanik] zly --id, poprawny zakres 1..8" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[mechanik] start, stanowisko_id=" << stanowisko_id << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[mechanik] blad ipc_init" << std::endl;
        return EXIT_FAILURE;
    }

#if defined(__unix__) || defined(__APPLE__)
    mechanik_zarejestruj_sygnaly();
    std::cout << "[mechanik] PID=" << getpid() << std::endl;
    mechanik_zapisz_pid(stanowisko_id);
#else
    std::cout << "[mechanik] system nie-Unix - sygnaly sa wylaczone (stub)" << std::endl;
#endif

    const int MAKS_ZLECEN = 200;

    for (int i = 0; i < MAKS_ZLECEN; ++i) {
        if (g_pozar) {
            std::cout << "[mechanik] pozar -> koniec" << std::endl;
            break;
        }
        if (!g_stanowisko_otwarte) {
            std::cout << "[mechanik] stanowisko zamkniete -> koniec" << std::endl;
            break;
        }

        Samochod s{};
        int id_klienta = 0;
        OfertaNaprawy oferta{};

        std::cout << "[mechanik] czekam na zlecenie (stanowisko=" << stanowisko_id << ")" << std::endl;

        if (serwis_ipc_odbierz_zlecenie(stanowisko_id, s, id_klienta, oferta) != SERWIS_IPC_OK) {
            std::cerr << "[mechanik] blad odbioru zlecenia" << std::endl;
            break;
        }

        std::cout << "[mechanik] zlecenie: id_klienta=" << id_klienta
                  << ", marka=" << s.marka
                  << ", czas_szac=" << oferta.czas
                  << ", koszt_szac=" << oferta.koszt
                  << ", tryb=" << (g_tryb_pracy == SERWIS_TRYB_NORMALNY ? "NORMALNY" : "PRZYSPIESZONY")
                  << std::endl;

        // 20% dodatkowych usterek + decyzja klienta
        int czas_dodatkowy = 0;
        int koszt_dodatkowy = 0;

        int los_usterka = serwis_losuj_int(&g_seed_mechanik, 0, 99);
        if (los_usterka < 20) {
            std::cout << "[mechanik] wykryto dodatkowe usterki" << std::endl;

            int los_klient = serwis_losuj_int(&g_seed_mechanik, 0, 99);
            if (serwis_klient_zgadza_sie_na_rozszerzenie(los_klient, 20)) {
                czas_dodatkowy = serwis_losuj_int(&g_seed_mechanik, 10, 60);
                koszt_dodatkowy = serwis_losuj_int(&g_seed_mechanik, 100, 500);

                std::cout << "[mechanik] klient zaakceptowal rozszerzenie: +"
                          << czas_dodatkowy << " min, +"
                          << koszt_dodatkowy << " zl" << std::endl;
            } else {
                std::cout << "[mechanik] klient odrzucil rozszerzenie" << std::endl;
            }
        }

        int rzeczywisty_czas = serwis_oblicz_czas_naprawy(oferta.czas, czas_dodatkowy, g_tryb_pracy);
        int koszt_koncowy = oferta.koszt + koszt_dodatkowy;

        std::cout << "[mechanik] raport: id_klienta=" << id_klienta
                  << ", czas=" << rzeczywisty_czas
                  << ", koszt=" << koszt_koncowy
                  << ", stanowisko=" << stanowisko_id
                  << std::endl;

        if (serwis_ipc_wyslij_raport(id_klienta, rzeczywisty_czas, koszt_koncowy, stanowisko_id, s) != SERWIS_IPC_OK) {
            std::cerr << "[mechanik] blad wysylki raportu" << std::endl;
        }

        if (g_zamknij_po_biezacej) {
            std::cout << "[mechanik] zamykam stanowisko po biezacej naprawie" << std::endl;
            g_stanowisko_otwarte = false;
        }
    }

    serwis_ipc_cleanup();
    std::cout << "[mechanik] koniec" << std::endl;
    return EXIT_SUCCESS;
}
