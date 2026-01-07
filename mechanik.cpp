// mechanik.cpp
#include <iostream>
#include <cstdlib>
#include "model.h"
#include "serwis_ipc.h"

#if defined(__unix__) || defined(__APPLE__)
    #include <csignal>   // sigaction, SIGUSR1, SIGUSR2, SIGTERM, SIGINT
    #include <unistd.h>  // getpid
#endif

// ================== ZMIENNE GLOBALNE MECHANIKA ==================

static SerwisTrybPracy g_tryb_pracy = SERWIS_TRYB_NORMALNY;
static bool g_stanowisko_otwarte = true;          // false -> nie przyjmujemy nowych zlecen
static bool g_zamknij_po_biezacej = false;        // po zakonczeniu biezacego zlecenia zamykamy
static bool g_pozar = false;                      // sygnal4 - natychmiastowe zakonczenie

// ================== OBSLUGA SYGNALOW (tylko Unix) ==================

#if defined(__unix__) || defined(__APPLE__)

// sygnal1 -> SIGUSR1  : zamknij stanowisko po biezacej naprawie
// sygnal2 -> SIGUSR2  : przyspiesz tryb pracy (50% czasu)
// sygnal3 -> SIGTERM  : powrot do trybu normalnego (tylko jesli wczesniej bylo przyspieszenie)
// sygnal4 -> SIGINT   : pozar - natychmiastowe zakonczenie pracy

void mechanik_signal_handler(int sig) {
    switch (sig) {
        case SIGUSR1:
            std::cout << "[mechanik] otrzymano sygnal1 (zamkniecie stanowiska po biezacej naprawie)"
                      << std::endl;
            g_zamknij_po_biezacej = true;
            break;

        case SIGUSR2:
            if (g_tryb_pracy == SERWIS_TRYB_NORMALNY) {
                std::cout << "[mechanik] otrzymano sygnal2 (przyspieszenie o 50%)" << std::endl;
                g_tryb_pracy = SERWIS_TRYB_PRZYSPIESZONY;
            } else {
                std::cout << "[mechanik] sygnal2 zignorowany (tryb juz przyspieszony)" << std::endl;
            }
            break;

        case SIGTERM:
            if (g_tryb_pracy == SERWIS_TRYB_PRZYSPIESZONY) {
                std::cout << "[mechanik] otrzymano sygnal3 (powrot do trybu normalnego)" << std::endl;
                g_tryb_pracy = SERWIS_TRYB_NORMALNY;
            } else {
                std::cout << "[mechanik] sygnal3 zignorowany (tryb juz normalny)" << std::endl;
            }
            break;

        case SIGINT:
            std::cout << "[mechanik] otrzymano sygnal4 (POZAR - natychmiastowe zakonczenie pracy)"
                      << std::endl;
            g_pozar = true;
            break;

        default:
            std::cout << "[mechanik] otrzymano nieznany sygnal: " << sig << std::endl;
            break;
    }
}

void mechanik_zarejestruj_sygnaly() {
    struct sigaction sa{};
    sa.sa_handler = mechanik_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, nullptr) == -1) {
        perror("[mechanik] blad sigaction SIGUSR1");
    }
    if (sigaction(SIGUSR2, &sa, nullptr) == -1) {
        perror("[mechanik] blad sigaction SIGUSR2");
    }
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        perror("[mechanik] blad sigaction SIGTERM");
    }
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        perror("[mechanik] blad sigaction SIGINT");
    }

    std::cout << "[mechanik] sygnaly zarejestrowane (SIGUSR1, SIGUSR2, SIGTERM, SIGINT)"
              << std::endl;
}

#endif // Unix

// ================== MAIN ==================

int main() {
    std::cout << "[mechanik] start procesu mechanika" << std::endl;

    if (serwis_ipc_init() != SERWIS_IPC_OK) {
        std::cerr << "[mechanik] blad inicjalizacji IPC" << std::endl;
        return EXIT_FAILURE;
    }

#if defined(__unix__) || defined(__APPLE__)
    mechanik_zarejestruj_sygnaly();
    std::cout << "[mechanik] moj PID = " << getpid()
              << " (uzyj kill, aby wyslac sygnaly na Linux)" << std::endl;
#else
    std::cout << "[mechanik] system nie-Unix - sygnaly sa wylaczone (stub)" << std::endl;
#endif

    const int MAKS_ZLECEN_DO_OBSLUGI = 100;

    for (int i = 0; i < MAKS_ZLECEN_DO_OBSLUGI; ++i) {
        if (g_pozar) {
            std::cout << "[mechanik] wykryto pozar - przerywam natychmiast petle" << std::endl;
            break;
        }

        if (!g_stanowisko_otwarte) {
            std::cout << "[mechanik] stanowisko zamkniete - nie przyjmuje nowych zlecen" << std::endl;
            break;
        }

        Samochod s{};
        int id_klienta = 0;
        OfertaNaprawy oferta{};

        std::cout << "[mechanik] czekam na zlecenie (iteracja " << (i + 1) << ")"
                  << std::endl;

        if (serwis_ipc_odbierz_zlecenie(s, id_klienta, oferta) != SERWIS_IPC_OK) {
            std::cerr << "[mechanik] blad odbierania zlecenia" << std::endl;
            break;
        }

        std::cout << "[mechanik] zlecenie: id_klienta=" << id_klienta
                  << ", marka=" << s.marka
                  << ", czas_szacowany=" << oferta.czas
                  << ", koszt_szacowany=" << oferta.koszt
                  << ", liczba_uslug=" << oferta.liczba_uslug
                  << ", tryb_pracy=" << (g_tryb_pracy == SERWIS_TRYB_NORMALNY ? "NORMALNY" : "PRZYSPIESZONY")
                  << std::endl;

        // Rzeczywisty czas na razie bierzemy z oferty, ale uwzgledniamy tryb pracy mechanika
        // (jesli kierownik przyspieszy, to czas realny moze byc krotszy)
        int rzeczywisty_czas = serwis_oblicz_czas_naprawy(oferta.czas, 0, g_tryb_pracy);

        // Koszt na razie taki jak w ofercie (pozniej doliczymy dodatkowe usterki 20%)
        int koszt_koncowy = oferta.koszt;

        std::cout << "[mechanik] wysylam raport: id_klienta=" << id_klienta
                  << ", rzeczywisty_czas=" << rzeczywisty_czas
                  << ", koszt_koncowy=" << koszt_koncowy
                  << std::endl;

        if (serwis_ipc_wyslij_raport(id_klienta, rzeczywisty_czas, koszt_koncowy, s) != SERWIS_IPC_OK) {
            std::cerr << "[mechanik] blad wysylania raportu" << std::endl;
        }

        // Jezeli dostalismy sygnal1, to po zakonczeniu tej naprawy zamykamy stanowisko
        if (g_zamknij_po_biezacej) {
            std::cout << "[mechanik] zamykam stanowisko po zakonczonej naprawie" << std::endl;
            g_stanowisko_otwarte = false;
        }
    }

    serwis_ipc_cleanup();

    std::cout << "[mechanik] koniec procesu mechanika" << std::endl;
    return EXIT_SUCCESS;
}
