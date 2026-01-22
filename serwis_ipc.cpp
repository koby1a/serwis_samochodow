#include "serwis_ipc.h"
#include <cstdio>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

static const int PERM = 0600;

static int q_zgl = -1;
static int q_zlec = -1;
static int q_rap = -1;

static int shm_id = -1;
static int sem_id = -1;
static SerwisStatystyki* shm = nullptr;

union semun { int val; struct semid_ds* buf; unsigned short* array; };

/** @brief ftok helper. */
static key_t k(const char* path, int id) {
    key_t kk = ftok(path, id);
    if (kk == -1) perror("[IPC] ftok");
    return kk;
}

/** @brief Lock semafora. */
static int sem_lock() {
    struct sembuf op{};
    op.sem_num = 0; op.sem_op = -1; op.sem_flg = 0;
    if (semop(sem_id, &op, 1) == -1) { perror("[IPC] semop lock"); return -1; }
    return 0;
}

/** @brief Unlock semafora. */
static int sem_unlock() {
    struct sembuf op{};
    op.sem_num = 0; op.sem_op = +1; op.sem_flg = 0;
    if (semop(sem_id, &op, 1) == -1) { perror("[IPC] semop unlock"); return -1; }
    return 0;
}

struct MsgZgl { long mtype; Samochod s; };
struct MsgZlec { long mtype; Zlecenie z; };
struct MsgRap { long mtype; Raport r; };

int serwis_ipc_init() {
    key_t kz  = k(".", 65);
    key_t kzl = k(".", 66);
    key_t kr  = k(".", 67);
    key_t ksh = k(".", 68);
    key_t kse = k(".", 69);
    if (kz==-1||kzl==-1||kr==-1||ksh==-1||kse==-1) return SERWIS_IPC_ERR;

    q_zgl = msgget(kz, IPC_CREAT | PERM);
    if (q_zgl == -1) { perror("[IPC] msgget zgl"); return SERWIS_IPC_ERR; }

    q_zlec = msgget(kzl, IPC_CREAT | PERM);
    if (q_zlec == -1) { perror("[IPC] msgget zlec"); return SERWIS_IPC_ERR; }

    q_rap = msgget(kr, IPC_CREAT | PERM);
    if (q_rap == -1) { perror("[IPC] msgget rap"); return SERWIS_IPC_ERR; }

    shm_id = shmget(ksh, sizeof(SerwisStatystyki), IPC_CREAT | PERM);
    if (shm_id == -1) { perror("[IPC] shmget"); return SERWIS_IPC_ERR; }

    void* p = shmat(shm_id, nullptr, 0);
    if (p == (void*)-1) { perror("[IPC] shmat"); return SERWIS_IPC_ERR; }
    shm = (SerwisStatystyki*)p;

    sem_id = semget(kse, 1, IPC_CREAT | PERM);
    if (sem_id == -1) { perror("[IPC] semget"); return SERWIS_IPC_ERR; }

    semun u{}; u.val = 1;
    if (semctl(sem_id, 0, SETVAL, u) == -1) perror("[IPC] semctl SETVAL");

    return SERWIS_IPC_OK;
}

void serwis_ipc_detach() {
    if (shm) { shmdt((void*)shm); shm = nullptr; }
}

void serwis_ipc_cleanup_all() {
    if (q_zgl  != -1) if (msgctl(q_zgl,  IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID zgl");
    if (q_zlec != -1) if (msgctl(q_zlec, IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID zlec");
    if (q_rap  != -1) if (msgctl(q_rap,  IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID rap");
    if (shm_id != -1) if (shmctl(shm_id, IPC_RMID, nullptr) == -1) perror("[IPC] shmctl RMID");
    if (sem_id != -1) if (semctl(sem_id, 0, IPC_RMID) == -1) perror("[IPC] semctl RMID");

    q_zgl = q_zlec = q_rap = -1;
    shm_id = sem_id = -1;
}

int serwis_ipc_send_zgl(const Samochod& s) {
    MsgZgl m{}; m.mtype = 1; m.s = s;
    if (msgsnd(q_zgl, &m, sizeof(MsgZgl)-sizeof(long), 0) == -1) { perror("[IPC] msgsnd zgl"); return SERWIS_IPC_ERR; }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_zgl(Samochod& s) {
    MsgZgl m{};
    if (msgrcv(q_zgl, &m, sizeof(MsgZgl)-sizeof(long), 1, 0) == -1) { perror("[IPC] msgrcv zgl"); return SERWIS_IPC_ERR; }
    s = m.s;
    return SERWIS_IPC_OK;
}

int serwis_ipc_send_zlec(const Zlecenie& z) {
    MsgZlec m{}; m.mtype = 100 + z.stanowisko_id; m.z = z;
    if (msgsnd(q_zlec, &m, sizeof(MsgZlec)-sizeof(long), 0) == -1) { perror("[IPC] msgsnd zlec"); return SERWIS_IPC_ERR; }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_zlec(int stanowisko_id, Zlecenie& z) {
    MsgZlec m{};
    long type = 100 + stanowisko_id;
    if (msgrcv(q_zlec, &m, sizeof(MsgZlec)-sizeof(long), type, 0) == -1) { perror("[IPC] msgrcv zlec"); return SERWIS_IPC_ERR; }
    z = m.z;
    return SERWIS_IPC_OK;
}

int serwis_ipc_send_rap(const Raport& r) {
    MsgRap m{}; m.mtype = 1; m.r = r;
    if (msgsnd(q_rap, &m, sizeof(MsgRap)-sizeof(long), 0) == -1) { perror("[IPC] msgsnd rap"); return SERWIS_IPC_ERR; }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_rap(Raport& r) {
    MsgRap m{};
    if (msgrcv(q_rap, &m, sizeof(MsgRap)-sizeof(long), 1, 0) == -1) { perror("[IPC] msgrcv rap"); return SERWIS_IPC_ERR; }
    r = m.r;
    return SERWIS_IPC_OK;
}

int serwis_stat_get(SerwisStatystyki& out) {
    if (!shm) return SERWIS_IPC_ERR;
    if (sem_lock() == 0) {
        out = *shm;
        sem_unlock();
        return SERWIS_IPC_OK;
    }
    return SERWIS_IPC_ERR;
}

void serwis_set_pozar(int v) {
    if (!shm) return;
    if (sem_lock() == 0) { shm->pozar = v ? 1 : 0; sem_unlock(); }
}

int serwis_get_pozar() {
    if (!shm) return 0;
    int v = 0;
    if (sem_lock() == 0) { v = shm->pozar; sem_unlock(); }
    return v;
}

void serwis_station_set_busy(int id, int busy, char marka, int kryt, int dodatkowe, int tryb) {
    if (!shm || id < 1 || id > 8) return;
    if (sem_lock() == 0) {
        shm->st[id].zajete = busy ? 1 : 0;
        shm->st[id].marka = marka;
        shm->st[id].krytyczna = kryt ? 1 : 0;
        shm->st[id].dodatkowe = dodatkowe ? 1 : 0;
        shm->st[id].tryb = tryb;
        sem_unlock();
    }
}

void serwis_station_inc_done(int id) {
    if (!shm || id < 1 || id > 8) return;
    if (sem_lock() == 0) { shm->st[id].obsluzone++; sem_unlock(); }
}

void serwis_station_set_closed(int id, int closed) {
    if (!shm || id < 1 || id > 8) return;
    if (sem_lock() == 0) { shm->st[id].zamkniete = closed ? 1 : 0; sem_unlock(); }
}

void serwis_req_close(int id, int v) {
    if (!shm || id < 1 || id > 8) return;
    if (sem_lock() == 0) { shm->req_close[id] = v ? 1 : 0; sem_unlock(); }
}

int serwis_get_req_close(int id) {
    if (!shm || id < 1 || id > 8) return 0;
    int v = 0;
    if (sem_lock() == 0) { v = shm->req_close[id]; sem_unlock(); }
    return v;
}
