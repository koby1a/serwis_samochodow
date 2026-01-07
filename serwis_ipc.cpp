// serwis_ipc.cpp
#include "serwis_ipc.h"
#include <iostream>

#if defined(__unix__) || defined(__APPLE__)
    #include <sys/ipc.h>
    #include <sys/msg.h>
    #include <cerrno>
    #include <cstring>
    #include <unistd.h>
#endif

// Id kolejek (System V)
static int g_msgid_zgloszenia = -1;
static int g_msgid_zlecenia = -1;
static int g_msgid_raporty = -1;

int serwis_ipc_init() {
#if defined(__unix__) || defined(__APPLE__)
    // Proste klucze (w praktyce na Linuxie lepiej ftok, ale tu trzymamy prosto)
    key_t key_zgl = 0x1111;
    key_t key_zlec = 0x2222;
    key_t key_rap = 0x3333;

    g_msgid_zgloszenia = msgget(key_zgl, IPC_CREAT | 0600);
    if (g_msgid_zgloszenia == -1) {
        perror("[IPC] blad msgget (zgloszenia)");
        return SERWIS_IPC_ERR;
    }

    g_msgid_zlecenia = msgget(key_zlec, IPC_CREAT | 0600);
    if (g_msgid_zlecenia == -1) {
        perror("[IPC] blad msgget (zlecenia)");
        return SERWIS_IPC_ERR;
    }

    g_msgid_raporty = msgget(key_rap, IPC_CREAT | 0600);
    if (g_msgid_raporty == -1) {
        perror("[IPC] blad msgget (raporty)");
        return SERWIS_IPC_ERR;
    }

    std::cout << "[IPC] inicjalizacja OK (System V msg queues)" << std::endl;
    return SERWIS_IPC_OK;
#else
    std::cout << "[IPC] inicjalizacja struktur IPC (stub, brak IPC na tym systemie)" << std::endl;
    return SERWIS_IPC_OK;
#endif
}

void serwis_ipc_cleanup() {
#if defined(__unix__) || defined(__APPLE__)
    // Usuwamy kolejki (w realu mozna zostawiac, ale wymagania mowia: sprzatac)
    if (g_msgid_zgloszenia != -1) {
        msgctl(g_msgid_zgloszenia, IPC_RMID, nullptr);
        g_msgid_zgloszenia = -1;
    }
    if (g_msgid_zlecenia != -1) {
        msgctl(g_msgid_zlecenia, IPC_RMID, nullptr);
        g_msgid_zlecenia = -1;
    }
    if (g_msgid_raporty != -1) {
        msgctl(g_msgid_raporty, IPC_RMID, nullptr);
        g_msgid_raporty = -1;
    }
    std::cout << "[IPC] sprzatanie OK" << std::endl;
#else
    std::cout << "[IPC] sprzatanie struktur IPC (stub, brak IPC na tym systemie)" << std::endl;
#endif
}

// ---------------- ZGLOSZENIA ----------------

int serwis_ipc_wyslij_zgloszenie(const Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)
    if (g_msgid_zgloszenia == -1) return SERWIS_IPC_ERR;

    MsgZgloszenie msg{};
    msg.mtype = SERWIS_MSGTYPE_ZGLOSZENIE;
    msg.s = s;

    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);
    if (msgsnd(g_msgid_zgloszenia, &msg, msgsz, 0) == -1) {
        perror("[IPC] blad msgsnd (zgloszenie)");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
#else
    std::cout << "[IPC] wyslij_zgloszenie (stub)" << std::endl;
    return SERWIS_IPC_OK;
#endif
}

int serwis_ipc_odbierz_zgloszenie(Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)
    if (g_msgid_zgloszenia == -1) return SERWIS_IPC_ERR;

    MsgZgloszenie msg{};
    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgrcv(g_msgid_zgloszenia, &msg, msgsz, SERWIS_MSGTYPE_ZGLOSZENIE, 0) == -1) {
        perror("[IPC] blad msgrcv (zgloszenie)");
        return SERWIS_IPC_ERR;
    }
    s = msg.s;
    return SERWIS_IPC_OK;
#else
    (void)s;
    std::cout << "[IPC] odbierz_zgloszenie (stub)" << std::endl;
    return SERWIS_IPC_ERR;
#endif
}

// ---------------- ZLECENIA (routing po mtype) ----------------

int serwis_ipc_wyslij_zlecenie(const Samochod& s,
                               int id_klienta,
                               int stanowisko_id,
                               const OfertaNaprawy& oferta) {
#if defined(__unix__) || defined(__APPLE__)
    if (g_msgid_zlecenia == -1) return SERWIS_IPC_ERR;
    if (stanowisko_id < 1 || stanowisko_id > 8) return SERWIS_IPC_ERR;

    MsgZlecenie msg{};
    msg.mtype = SERWIS_MTYPE_ZLECENIE_BASE + stanowisko_id;
    msg.s = s;
    msg.id_klienta = id_klienta;
    msg.stanowisko_id = stanowisko_id;

    msg.liczba_uslug = oferta.liczba_uslug;
    for (int i = 0; i < 10; ++i) msg.uslugi_id[i] = oferta.uslugi_id[i];
    msg.koszt_szacowany = oferta.koszt;
    msg.czas_szacowany = oferta.czas;

    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);
    if (msgsnd(g_msgid_zlecenia, &msg, msgsz, 0) == -1) {
        perror("[IPC] blad msgsnd (zlecenie)");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
#else
    std::cout << "[IPC] wyslij_zlecenie (stub). id_klienta=" << id_klienta
              << ", stanowisko=" << stanowisko_id << std::endl;
    return SERWIS_IPC_OK;
#endif
}

int serwis_ipc_odbierz_zlecenie(int stanowisko_id,
                                Samochod& s,
                                int& id_klienta,
                                OfertaNaprawy& oferta) {
#if defined(__unix__) || defined(__APPLE__)
    if (g_msgid_zlecenia == -1) return SERWIS_IPC_ERR;
    if (stanowisko_id < 1 || stanowisko_id > 8) return SERWIS_IPC_ERR;

    MsgZlecenie msg{};
    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    long typ = SERWIS_MTYPE_ZLECENIE_BASE + stanowisko_id;
    if (msgrcv(g_msgid_zlecenia, &msg, msgsz, typ, 0) == -1) {
        perror("[IPC] blad msgrcv (zlecenie)");
        return SERWIS_IPC_ERR;
    }

    s = msg.s;
    id_klienta = msg.id_klienta;

    oferta.liczba_uslug = msg.liczba_uslug;
    for (int i = 0; i < 10; ++i) oferta.uslugi_id[i] = msg.uslugi_id[i];
    oferta.koszt = msg.koszt_szacowany;
    oferta.czas = msg.czas_szacowany;

    return SERWIS_IPC_OK;
#else
    (void)stanowisko_id; (void)s; (void)id_klienta; (void)oferta;
    std::cout << "[IPC] odbierz_zlecenie (stub)" << std::endl;
    return SERWIS_IPC_ERR;
#endif
}

// ---------------- RAPORTY ----------------

int serwis_ipc_wyslij_raport(int id_klienta,
                             int rzeczywisty_czas,
                             int koszt_koncowy,
                             int stanowisko_id,
                             const Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)
    if (g_msgid_raporty == -1) return SERWIS_IPC_ERR;

    MsgRaport msg{};
    msg.mtype = SERWIS_MSGTYPE_RAPORT;
    msg.id_klienta = id_klienta;
    msg.rzeczywisty_czas = rzeczywisty_czas;
    msg.koszt_koncowy = koszt_koncowy;
    msg.stanowisko_id = stanowisko_id;
    msg.s = s;

    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);
    if (msgsnd(g_msgid_raporty, &msg, msgsz, 0) == -1) {
        perror("[IPC] blad msgsnd (raport)");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
#else
    std::cout << "[IPC] wyslij_raport (stub). id_klienta=" << id_klienta
              << ", stanowisko=" << stanowisko_id << std::endl;
    return SERWIS_IPC_OK;
#endif
}

int serwis_ipc_odbierz_raport(int& id_klienta,
                              int& rzeczywisty_czas,
                              int& koszt_koncowy,
                              int& stanowisko_id,
                              Samochod& s) {
#if defined(__unix__) || defined(__APPLE__)
    if (g_msgid_raporty == -1) return SERWIS_IPC_ERR;

    MsgRaport msg{};
    size_t msgsz = sizeof(msg) - sizeof(msg.mtype);

    if (msgrcv(g_msgid_raporty, &msg, msgsz, SERWIS_MSGTYPE_RAPORT, 0) == -1) {
        perror("[IPC] blad msgrcv (raport)");
        return SERWIS_IPC_ERR;
    }

    id_klienta = msg.id_klienta;
    rzeczywisty_czas = msg.rzeczywisty_czas;
    koszt_koncowy = msg.koszt_koncowy;
    stanowisko_id = msg.stanowisko_id;
    s = msg.s;

    return SERWIS_IPC_OK;
#else
    (void)id_klienta; (void)rzeczywisty_czas; (void)koszt_koncowy; (void)stanowisko_id; (void)s;
    std::cout << "[IPC] odbierz_raport (stub)" << std::endl;
    return SERWIS_IPC_ERR;
#endif
}
