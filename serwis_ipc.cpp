// serwis_ipc.cpp
#include "serwis_ipc.h"
#include "komunikaty.h"
#include <iostream>

#if defined(__unix__) || defined(__APPLE__)

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstring>

// Minimalne prawa dostepu
static const int SERWIS_PERM = 0600;

// 3 kolejki: zgloszenia / zlecenia / raporty
static int g_q_zgl = -1;
static int g_q_zlec = -1;
static int g_q_rap = -1;

// SHM + SEM dla statystyk
static int g_shm_id = -1;
static int g_sem_id = -1;
static SerwisStatystyki* g_stats = nullptr;

static key_t serwis_key(const char* path, int proj_id) {
    key_t k = ftok(path, proj_id);
    if (k == -1) {
        perror("[IPC] ftok");
    }
    return k;
}

// semun nie jest standardowo zdefiniowany na kazdym systemie
union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

static int sem_lock() {
    if (g_sem_id < 0) return -1;
    struct sembuf op{};
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(g_sem_id, &op, 1) == -1) {
        perror("[IPC] semop lock");
        return -1;
    }
    return 0;
}

static int sem_unlock() {
    if (g_sem_id < 0) return -1;
    struct sembuf op{};
    op.sem_num = 0;
    op.sem_op = +1;
    op.sem_flg = 0;
    if (semop(g_sem_id, &op, 1) == -1) {
        perror("[IPC] semop unlock");
        return -1;
    }
    return 0;
}

static void stat_inc(int* field) {
    if (!g_stats || !field) return;
    if (sem_lock() == 0) {
        (*field)++;
        sem_unlock();
    }
}

int serwis_ipc_init() {
    // Uzywamy pliku biezacego katalogu jako “seed” do ftok:
    // W realu mozna dac np. "./" i roznicowac proj_id.
    key_t k_zgl = serwis_key(".", 65);
    key_t k_zlec = serwis_key(".", 66);
    key_t k_rap = serwis_key(".", 67);
    key_t k_shm = serwis_key(".", 68);
    key_t k_sem = serwis_key(".", 69);

    if (k_zgl == -1 || k_zlec == -1 || k_rap == -1 || k_shm == -1 || k_sem == -1) {
        return SERWIS_IPC_ERR;
    }

    // Kolejki
    g_q_zgl = msgget(k_zgl, IPC_CREAT | SERWIS_PERM);
    if (g_q_zgl == -1) { perror("[IPC] msgget zgl"); return SERWIS_IPC_ERR; }

    g_q_zlec = msgget(k_zlec, IPC_CREAT | SERWIS_PERM);
    if (g_q_zlec == -1) { perror("[IPC] msgget zlec"); return SERWIS_IPC_ERR; }

    g_q_rap = msgget(k_rap, IPC_CREAT | SERWIS_PERM);
    if (g_q_rap == -1) { perror("[IPC] msgget rap"); return SERWIS_IPC_ERR; }

    // SHM
    g_shm_id = shmget(k_shm, sizeof(SerwisStatystyki), IPC_CREAT | SERWIS_PERM);
    if (g_shm_id == -1) { perror("[IPC] shmget"); return SERWIS_IPC_ERR; }

    void* p = shmat(g_shm_id, nullptr, 0);
    if (p == (void*)-1) { perror("[IPC] shmat"); g_stats = nullptr; return SERWIS_IPC_ERR; }
    g_stats = static_cast<SerwisStatystyki*>(p);

    // SEM (1 semafor binarny)
    g_sem_id = semget(k_sem, 1, IPC_CREAT | SERWIS_PERM);
    if (g_sem_id == -1) { perror("[IPC] semget"); return SERWIS_IPC_ERR; }

    // Inicjalizacja semafora do 1 (tylko gdy nowy) - bezpieczne: ustawiamy zawsze
    semun u{};
    u.val = 1;
    if (semctl(g_sem_id, 0, SETVAL, u) == -1) {
        // Jesli ktos nie pozwala, to logujemy, ale nie przerywamy (bo moze byc juz ustawiony)
        perror("[IPC] semctl SETVAL");
    }

    return SERWIS_IPC_OK;
}

void serwis_ipc_cleanup() {
    // Odpiecie od shm
    if (g_stats) {
        shmdt(reinterpret_cast<void*>(g_stats));
        g_stats = nullptr;
    }
}

void serwis_ipc_cleanup_all() {
    // Uwaga: wywolywac tylko w orchestratorze, gdy juz dzieci nie zyja
    if (g_q_zgl != -1) {
        if (msgctl(g_q_zgl, IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID zgl");
    }
    if (g_q_zlec != -1) {
        if (msgctl(g_q_zlec, IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID zlec");
    }
    if (g_q_rap != -1) {
        if (msgctl(g_q_rap, IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID rap");
    }
    if (g_shm_id != -1) {
        if (shmctl(g_shm_id, IPC_RMID, nullptr) == -1) perror("[IPC] shmctl RMID");
    }
    if (g_sem_id != -1) {
        if (semctl(g_sem_id, 0, IPC_RMID) == -1) perror("[IPC] semctl RMID");
    }

    g_q_zgl = g_q_zlec = g_q_rap = -1;
    g_shm_id = g_sem_id = -1;
}

int serwis_ipc_wyslij_zgloszenie(const Samochod& s) {
    if (g_q_zgl == -1) return SERWIS_IPC_ERR;
    MsgZgloszenie m{};
    m.mtype = 1;
    m.s = s;

    if (msgsnd(g_q_zgl, &m, sizeof(MsgZgloszenie) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd zgl");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_odbierz_zgloszenie(Samochod& s) {
    if (g_q_zgl == -1) return SERWIS_IPC_ERR;
    MsgZgloszenie m{};
    if (msgrcv(g_q_zgl, &m, sizeof(MsgZgloszenie) - sizeof(long), 1, 0) == -1) {
        perror("[IPC] msgrcv zgl");
        return SERWIS_IPC_ERR;
    }
    s = m.s;
    return SERWIS_IPC_OK;
}

int serwis_ipc_wyslij_zlecenie(const Samochod& s, int id_klienta, int stanowisko_id, const OfertaNaprawy& oferta) {
    if (g_q_zlec == -1) return SERWIS_IPC_ERR;
    MsgZlecenie m{};
    m.mtype = 100 + stanowisko_id;
    m.stanowisko_id = stanowisko_id;
    m.id_klienta = id_klienta;
    m.s = s;
    m.oferta = oferta;

    if (msgsnd(g_q_zlec, &m, sizeof(MsgZlecenie) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd zlec");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_odbierz_zlecenie(int stanowisko_id, Samochod& s, int& id_klienta, OfertaNaprawy& oferta) {
    if (g_q_zlec == -1) return SERWIS_IPC_ERR;
    MsgZlecenie m{};
    long type = 100 + stanowisko_id;

    if (msgrcv(g_q_zlec, &m, sizeof(MsgZlecenie) - sizeof(long), type, 0) == -1) {
        perror("[IPC] msgrcv zlec");
        return SERWIS_IPC_ERR;
    }

    s = m.s;
    id_klienta = m.id_klienta;
    oferta = m.oferta;
    return SERWIS_IPC_OK;
}

int serwis_ipc_wyslij_raport(int id_klienta, int rzeczywisty_czas, int koszt_koncowy, int stanowisko_id, const Samochod& s) {
    if (g_q_rap == -1) return SERWIS_IPC_ERR;
    MsgRaport m{};
    m.mtype = 1;
    m.id_klienta = id_klienta;
    m.stanowisko_id = stanowisko_id;
    m.s = s;
    m.rzeczywisty_czas = rzeczywisty_czas;
    m.koszt_koncowy = koszt_koncowy;

    if (msgsnd(g_q_rap, &m, sizeof(MsgRaport) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd raport");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_odbierz_raport(int& id_klienta, int& rzeczywisty_czas, int& koszt_koncowy, int& stanowisko_id, Samochod& s) {
    if (g_q_rap == -1) return SERWIS_IPC_ERR;
    MsgRaport m{};
    if (msgrcv(g_q_rap, &m, sizeof(MsgRaport) - sizeof(long), 1, 0) == -1) {
        perror("[IPC] msgrcv raport");
        return SERWIS_IPC_ERR;
    }

    id_klienta = m.id_klienta;
    rzeczywisty_czas = m.rzeczywisty_czas;
    koszt_koncowy = m.koszt_koncowy;
    stanowisko_id = m.stanowisko_id;
    s = m.s;
    return SERWIS_IPC_OK;
}

// -------- Statystyki --------

int serwis_stat_pobierz(SerwisStatystyki& out) {
    if (!g_stats) return SERWIS_IPC_ERR;
    if (sem_lock() == 0) {
        out = *g_stats;
        sem_unlock();
        return SERWIS_IPC_OK;
    }
    return SERWIS_IPC_ERR;
}

void serwis_stat_inc_przyjete() { if (g_stats) stat_inc(&g_stats->przyjete_zgloszenia); }
void serwis_stat_inc_odrzucone_marka() { if (g_stats) stat_inc(&g_stats->odrzucone_marka); }
void serwis_stat_inc_odrzucone_poza_godzinami() { if (g_stats) stat_inc(&g_stats->odrzucone_poza_godzinami); }
void serwis_stat_inc_odrzucone_oferta() { if (g_stats) stat_inc(&g_stats->odrzucone_oferta); }
void serwis_stat_inc_brak_stanowiska() { if (g_stats) stat_inc(&g_stats->brak_stanowiska); }
void serwis_stat_inc_wyslane_zlecenia() { if (g_stats) stat_inc(&g_stats->wyslane_zlecenia); }
void serwis_stat_inc_wykonane_naprawy() { if (g_stats) stat_inc(&g_stats->wykonane_naprawy); }
void serwis_stat_inc_platnosci() { if (g_stats) stat_inc(&g_stats->platnosci); }

#else

// ---------------- Windows / non-Unix stub ----------------

int serwis_ipc_init() {
    std::cout << "[IPC] inicjalizacja struktur IPC (stub, brak IPC na tym systemie)" << std::endl;
    return SERWIS_IPC_OK;
}

void serwis_ipc_cleanup() {
    std::cout << "[IPC] sprzatanie struktur IPC (stub)" << std::endl;
}

void serwis_ipc_cleanup_all() {
    std::cout << "[IPC] sprzatanie ALL (stub)" << std::endl;
}

int serwis_ipc_wyslij_zgloszenie(const Samochod&) { return SERWIS_IPC_OK; }
int serwis_ipc_odbierz_zgloszenie(Samochod&) { return SERWIS_IPC_ERR; }

int serwis_ipc_wyslij_zlecenie(const Samochod&, int, int, const OfertaNaprawy&) { return SERWIS_IPC_OK; }
int serwis_ipc_odbierz_zlecenie(int, Samochod&, int&, OfertaNaprawy&) { return SERWIS_IPC_ERR; }

int serwis_ipc_wyslij_raport(int, int, int, int, const Samochod&) { return SERWIS_IPC_OK; }
int serwis_ipc_odbierz_raport(int&, int&, int&, int&, Samochod&) { return SERWIS_IPC_ERR; }

int serwis_stat_pobierz(SerwisStatystyki&) { return SERWIS_IPC_ERR; }
void serwis_stat_inc_przyjete() {}
void serwis_stat_inc_odrzucone_marka() {}
void serwis_stat_inc_odrzucone_poza_godzinami() {}
void serwis_stat_inc_odrzucone_oferta() {}
void serwis_stat_inc_brak_stanowiska() {}
void serwis_stat_inc_wyslane_zlecenia() {}
void serwis_stat_inc_wykonane_naprawy() {}
void serwis_stat_inc_platnosci() {}

#endif
