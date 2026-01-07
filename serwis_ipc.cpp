// serwis_ipc.cpp
#include <iostream>
#include "serwis_ipc.h"

#if defined(__unix__) || defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/msg.h>
    #include <cerrno>
    #include <cstdio>   // perror
#endif

// Globalne identyfikatory kolejek
static int g_msgid_zgloszenia = -1;  // kolejka zgloszen (kierowca -> pracownik)
static int g_msgid_zlecenia   = -1;  // kolejka zlecen (pracownik -> mechanik)
static int g_msgid_raporty    = -1;  // kolejka raportow (mechanik -> kasjer)

int serwis_ipc_init() {
#if defined(__unix__) || defined(__APPLE__)

    std::cout << "[IPC] inicjalizacja struktur IPC (Unix)" << std::endl;

    // KOLEJKA ZGLOSZEN (ftok z proj_id 'Z')
    if (g_msgid_zgloszenia == -1) {
        key_t key_zgloszenia = ftok(".", 'Z');
        if (key_zgloszenia == -1) {
            perror("[IPC] blad ftok dla kolejki zgloszen");
            return SERWIS_IPC_ERR;
        }

        int msgflg = IPC_CREAT | 0600;
        int msgid = msgget(key_zgloszenia, msgflg);
        if (msgid == -1) {
            perror("[IPC] blad msgget dla kolejki zgloszen");
            return SERWIS_IPC_ERR;
        }

        g_msgid_zgloszenia = msgid;
        std::cout << "[IPC] kolejka zgloszen msgid = " << g_msgid_zgloszenia << std::endl;
    }

    // KOLEJKA ZLECEN (ftok z proj_id 'N')
    if (g_msgid_zlecenia == -1) {
        key_t key_zlecenia = ftok(".", 'N');
        if (key_zlecenia == -1) {
            perror("[IPC] blad ftok dla kolejki zlecen");
            return SERWIS_IPC_ERR;
        }

        int msgflg = IPC_CREAT | 0600;
        int msgid = msgget(key_zlecenia, msgflg);
        if (msgid == -1) {
            perror("[IPC] blad msgget dla kolejki zlecen");
            return SERWIS_IPC_ERR;
        }

        g_msgid_zlecenia = msgid;
        std::cout << "[IPC] kolejka zlecen msgid = " << g_msgid_zlecenia << std::endl;
    }

    // KOLEJKA RAPORTOW (ftok z proj_id 'R')
    if (g_msgid_raporty == -1) {
        key_t key_raporty = ftok(".", 'R');
        if (key_raporty == -1) {
            perror("[IPC] blad ftok dla kolejki raportow");
            return SERWIS_IPC_ERR;
        }

        int msgflg = IPC_CREAT | 0600;
        int msgid = msgget(key_raporty, msgflg);
        if (msgid == -1) {
            perror("[IPC] blad msgget dla kolejki raportow");
            return SERWIS_IPC_ERR;
        }

        g_msgid_raporty = msgid;
        std::cout << "[IPC] kolejka raportow msgid = " << g_msgid_raporty << std::endl;
    }

    return SERWIS_IPC_OK;

#else

    std::cout << "[IPC] inicjalizacja struktur IPC (stub, brak IPC na tym systemie)" << std::endl;
    g_msgid_zgloszenia = -1;
    g_msgid_zlecenia   = -1;
    g_msgid_raporty    = -1;
    return SERWIS_IPC_OK;

#endif
}

void serwis_ipc_cleanup() {
#if defined(__unix__) || defined(__APPLE__)

    std::cout << "[IPC] sprzatanie struktur IPC (Unix)" << std::endl;

    if (g_msgid_zgloszenia != -1) {
        if (msgctl(g_msgid_zgloszenia, IPC_RMID, nullptr) == -1) {
            perror("[IPC] blad msgctl IPC_RMID dla kolejki zgloszen");
        } else {
            std::cout << "[IPC] kolejka zgloszen usunieta" << std::endl;
        }
        g_msgid_zgloszenia = -1;
    }

    if (g_msgid_zlecenia != -1) {
        if (msgctl(g_msgid_zlecenia, IPC_RMID, nullptr) == -1) {
            perror("[IPC] blad msgctl IPC_RMID dla kolejki zlecen");
        } else {
            std::cout << "[IPC] kolejka zlecen usunieta" << std::endl;
        }
        g_msgid_zlecenia = -1;
    }

    if (g_msgid_raporty != -1) {
        if (msgctl(g_msgid_raporty, IPC_RMID, nullptr) == -1) {
            perror("[IPC] blad msgctl IPC_RMID dla kolejki raportow");
        } else {
            std::cout << "[IPC] kolejka raportow usunieta" << std::endl;
        }
        g_msgid_raporty = -1;
    }

#else

    std::cout << "[IPC] sprzatanie struktur IPC (stub, brak IPC na tym systemie)" << std::endl;

#endif
}

int serwis_ipc_get_msgid_zgloszenia() {
    return g_msgid_zgloszenia;
}

// ================== ZGLOSZENIA ==================

int serwis_ipc_wyslij_zgloszenie(const Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_zgloszenia == -1) {
        std::cerr << "[IPC] wyslij_zgloszenie: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgZgloszenie msg{};
    msg.mtype = SERWIS_MSGTYPE_ZGLOSZENIE;
    msg.s = s;

    size_t msgsz = sizeof(msg.s);

    if (msgsnd(g_msgid_zgloszenia, &msg, msgsz, 0) == -1) {
        perror("[IPC] blad msgsnd (wyslij_zgloszenie)");
        return SERWIS_IPC_ERR;
    }

    std::cout << "[IPC] wyslano zgloszenie (marka = " << s.marka << ")" << std::endl;
    return SERWIS_IPC_OK;

#else

    std::cout << "[IPC] wyslij_zgloszenie (stub, brak IPC na tym systemie). "
                 "Symulacja wyslania samochodu marki "
              << s.marka << std::endl;
    return SERWIS_IPC_OK;

#endif
}

int serwis_ipc_odbierz_zgloszenie(Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_zgloszenia == -1) {
        std::cerr << "[IPC] odbierz_zgloszenie: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgZgloszenie msg{};
    size_t msgsz = sizeof(msg.s);

    if (msgrcv(g_msgid_zgloszenia, &msg, msgsz,
               SERWIS_MSGTYPE_ZGLOSZENIE, 0) == -1) {
        perror("[IPC] blad msgrcv (odbierz_zgloszenie)");
        return SERWIS_IPC_ERR;
    }

    s = msg.s;
    std::cout << "[IPC] odebrano zgloszenie (marka = " << s.marka << ")" << std::endl;
    return SERWIS_IPC_OK;

#else

    std::cout << "[IPC] odbierz_zgloszenie (stub, brak IPC na tym systemie)" << std::endl;
    return SERWIS_IPC_ERR;

#endif
}

// ================== ZLECENIA ==================

int serwis_ipc_wyslij_zlecenie(const Samochod& s,
                               int przewidywany_czas,
                               int id_klienta) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_zlecenia == -1) {
        std::cerr << "[IPC] wyslij_zlecenie: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgZlecenie msg{};
    msg.mtype = SERWIS_MSGTYPE_ZLECENIE;
    msg.s = s;
    msg.przewidywany_czas = przewidywany_czas;
    msg.id_klienta = id_klienta;

    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgsnd(g_msgid_zlecenia, &msg, msgsz, 0) == -1) {
        perror("[IPC] blad msgsnd (wyslij_zlecenie)");
        return SERWIS_IPC_ERR;
    }

    std::cout << "[IPC] wyslano zlecenie (id_klienta = "
              << id_klienta
              << ", marka = " << s.marka
              << ", przewidywany_czas = " << przewidywany_czas
              << ")" << std::endl;

    return SERWIS_IPC_OK;

#else

    std::cout << "[IPC] wyslij_zlecenie (stub, brak IPC na tym systemie). "
                 "Symulacja wyslania zlecenia dla klienta "
              << id_klienta << std::endl;
    return SERWIS_IPC_OK;

#endif
}

int serwis_ipc_odbierz_zlecenie(Samochod& s,
                                int& przewidywany_czas,
                                int& id_klienta) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_zlecenia == -1) {
        std::cerr << "[IPC] odbierz_zlecenie: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgZlecenie msg{};
    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgrcv(g_msgid_zlecenia, &msg, msgsz,
               SERWIS_MSGTYPE_ZLECENIE, 0) == -1) {
        perror("[IPC] blad msgrcv (odbierz_zlecenie)");
        return SERWIS_IPC_ERR;
    }

    s = msg.s;
    przewidywany_czas = msg.przewidywany_czas;
    id_klienta = msg.id_klienta;

    std::cout << "[IPC] odebrano zlecenie (id_klienta = "
              << id_klienta
              << ", marka = " << s.marka
              << ", przewidywany_czas = " << przewidywany_czas
              << ")" << std::endl;

    return SERWIS_IPC_OK;

#else

    std::cout << "[IPC] odbierz_zlecenie (stub, brak IPC na tym systemie)" << std::endl;
    return SERWIS_IPC_ERR;

#endif
}
int serwis_ipc_wyslij_zlecenie(const Samochod& s,
                               int id_klienta,
                               const OfertaNaprawy& oferta) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_zlecenia == -1) {
        std::cerr << "[IPC] wyslij_zlecenie: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgZlecenie msg{};
    msg.mtype = SERWIS_MSGTYPE_ZLECENIE;
    msg.s = s;
    msg.id_klienta = id_klienta;

    msg.liczba_uslug = oferta.liczba_uslug;
    for (int i = 0; i < 10; ++i) {
        msg.uslugi_id[i] = oferta.uslugi_id[i];
    }
    msg.koszt_szacowany = oferta.koszt;
    msg.czas_szacowany = oferta.czas;

    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgsnd(g_msgid_zlecenia, &msg, msgsz, 0) == -1) {
        perror("[IPC] blad msgsnd (wyslij_zlecenie)");
        return SERWIS_IPC_ERR;
    }

    std::cout << "[IPC] wyslano zlecenie (id_klienta = " << id_klienta
              << ", marka = " << s.marka
              << ", czas_szacowany = " << msg.czas_szacowany
              << ", koszt_szacowany = " << msg.koszt_szacowany
              << ", liczba_uslug = " << msg.liczba_uslug
              << ")" << std::endl;

    return SERWIS_IPC_OK;

#else
    std::cout << "[IPC] wyslij_zlecenie (stub). id_klienta=" << id_klienta
              << ", czas=" << oferta.czas << ", koszt=" << oferta.koszt << std::endl;
    return SERWIS_IPC_OK;
#endif
}

int serwis_ipc_odbierz_zlecenie(Samochod& s,
                                int& id_klienta,
                                OfertaNaprawy& oferta) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_zlecenia == -1) {
        std::cerr << "[IPC] odbierz_zlecenie: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgZlecenie msg{};
    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgrcv(g_msgid_zlecenia, &msg, msgsz,
               SERWIS_MSGTYPE_ZLECENIE, 0) == -1) {
        perror("[IPC] blad msgrcv (odbierz_zlecenie)");
        return SERWIS_IPC_ERR;
               }

    s = msg.s;
    id_klienta = msg.id_klienta;

    oferta.liczba_uslug = msg.liczba_uslug;
    for (int i = 0; i < 10; ++i) {
        oferta.uslugi_id[i] = msg.uslugi_id[i];
    }
    oferta.koszt = msg.koszt_szacowany;
    oferta.czas = msg.czas_szacowany;

    std::cout << "[IPC] odebrano zlecenie (id_klienta = " << id_klienta
              << ", marka = " << s.marka
              << ", czas_szacowany = " << oferta.czas
              << ", koszt_szacowany = " << oferta.koszt
              << ", liczba_uslug = " << oferta.liczba_uslug
              << ")" << std::endl;

    return SERWIS_IPC_OK;

#else
    std::cout << "[IPC] odbierz_zlecenie (stub)" << std::endl;
    return SERWIS_IPC_ERR;
#endif
}


// ================== RAPORTY ==================

int serwis_ipc_wyslij_raport(int id_klienta,
                             int rzeczywisty_czas,
                             int koszt_koncowy,
                             const Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_raporty == -1) {
        std::cerr << "[IPC] wyslij_raport: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgRaport msg{};
    msg.mtype = SERWIS_MSGTYPE_RAPORT;
    msg.id_klienta = id_klienta;
    msg.rzeczywisty_czas = rzeczywisty_czas;
    msg.koszt_koncowy = koszt_koncowy;
    msg.s = s;

    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgsnd(g_msgid_raporty, &msg, msgsz, 0) == -1) {
        perror("[IPC] blad msgsnd (wyslij_raport)");
        return SERWIS_IPC_ERR;
    }

    std::cout << "[IPC] wyslano raport (id_klienta = "
              << id_klienta
              << ", rzeczywisty_czas = " << rzeczywisty_czas
              << ", koszt_koncowy = " << koszt_koncowy
              << ")" << std::endl;

    return SERWIS_IPC_OK;

#else

    std::cout << "[IPC] wyslij_raport (stub, brak IPC na tym systemie). "
                 "Symulacja wyslania raportu dla klienta "
              << id_klienta << std::endl;
    return SERWIS_IPC_OK;

#endif
}

int serwis_ipc_odbierz_raport(int& id_klienta,
                              int& rzeczywisty_czas,
                              int& koszt_koncowy,
                              Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)

    if (g_msgid_raporty == -1) {
        std::cerr << "[IPC] odbierz_raport: kolejka nie jest zainicjalizowana" << std::endl;
        return SERWIS_IPC_ERR;
    }

    MsgRaport msg{};
    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgrcv(g_msgid_raporty, &msg, msgsz,
               SERWIS_MSGTYPE_RAPORT, 0) == -1) {
        perror("[IPC] blad msgrcv (odbierz_raport)");
        return SERWIS_IPC_ERR;
    }

    id_klienta = msg.id_klienta;
    rzeczywisty_czas = msg.rzeczywisty_czas;
    koszt_koncowy = msg.koszt_koncowy;
    s = msg.s;

    std::cout << "[IPC] odebrano raport (id_klienta = "
              << id_klienta
              << ", rzeczywisty_czas = " << rzeczywisty_czas
              << ", koszt_koncowy = " << koszt_koncowy
              << ")" << std::endl;

    return SERWIS_IPC_OK;

#else

    std::cout << "[IPC] odbierz_raport (stub, brak IPC na tym systemie)" << std::endl;
    return SERWIS_IPC_ERR;

#endif
}
