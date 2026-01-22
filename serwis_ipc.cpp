/** @file serwis_ipc.cpp */

#include "serwis_ipc.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <unistd.h>
#include <libgen.h>
#include <limits.h>

static const int PERM = 0600;

static int q_zgl  = -1;
static int q_zlec = -1;
static int q_rap  = -1;
static int q_kasa = -1;
static int q_ext  = -1;

static int shm_id = -1;
static int sem_id = -1;
static SerwisStatystyki* shm = nullptr;

union semun { int val; struct semid_ds* buf; unsigned short* array; };

struct MsgZgl  { long mtype; Samochod s; };
struct MsgZlec { long mtype; Zlecenie z; };
struct MsgRap  { long mtype; Raport r; };
struct MsgExtReq  { long mtype; SerwisExtraReq r; };
struct MsgExtResp { long mtype; SerwisExtraResp r; };

/** @brief Pobiera katalog pliku wykonywalnego (stabilne ftok). */
static const char* exe_dir() {
    static char dir_buf[PATH_MAX];
    static int inited = 0;
    if (inited) return dir_buf;

    char path_buf[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", path_buf, sizeof(path_buf) - 1);
    if (n <= 0) {
        std::strcpy(dir_buf, ".");
        inited = 1;
        return dir_buf;
    }

    path_buf[n] = '\0';
    std::strncpy(dir_buf, path_buf, sizeof(dir_buf) - 1);
    dir_buf[sizeof(dir_buf) - 1] = '\0';

    char* d = dirname(dir_buf);
    if (!d) {
        std::strcpy(dir_buf, ".");
    } else {
        std::strncpy(dir_buf, d, sizeof(dir_buf) - 1);
        dir_buf[sizeof(dir_buf) - 1] = '\0';
    }

    inited = 1;
    return dir_buf;
}

/** @brief Tworzy klucz ftok na bazie katalogu exe (stabilne). */
static key_t key_make(int id) {
    key_t kk = ftok(exe_dir(), id);
    if (kk == -1) perror("[IPC] ftok");
    return kk;
}

/** @brief Lock semafora (mutex). */
static int sem_lock() {
    if (sem_id < 0) return -1;

    struct sembuf op{};
    op.sem_num = 0;
    op.sem_op  = -1;
    op.sem_flg = 0;

    if (semop(sem_id, &op, 1) == -1) {
        if (errno == EINTR) return -2;

        if (errno == EINVAL) {
            static int printed = 0;
            if (!printed) {
                printed = 1;
                perror("[IPC] semop lock (EINVAL)");
            }
            return -3;
        }

        perror("[IPC] semop lock");
        return -1;
    }

    return 0;
}

/** @brief Unlock semafora (mutex). */
static int sem_unlock() {
    if (sem_id < 0) return -1;

    struct sembuf op{};
    op.sem_num = 0;
    op.sem_op  = +1;
    op.sem_flg = 0;

    if (semop(sem_id, &op, 1) == -1) {
        if (errno == EINTR) return -2;

        if (errno == EINVAL) {
            static int printed = 0;
            if (!printed) {
                printed = 1;
                perror("[IPC] semop unlock (EINVAL)");
            }
            return -3;
        }

        perror("[IPC] semop unlock");
        return -1;
    }

    return 0;
}

/** @brief Zeruje SHM na start nowej symulacji. */
static void shm_reset_all() {
    if (!shm) return;

    shm->pozar = 0;
    shm->sim_time_min = 8 * 60;

    for (int i = 1; i <= 8; ++i) {
        shm->st[i].zajete = 0;
        shm->st[i].zamkniete = 0;
        shm->st[i].obsluzone = 0;
        shm->st[i].marka = '-';
        shm->st[i].krytyczna = 0;
        shm->st[i].dodatkowe = 0;
        shm->st[i].tryb = SERWIS_TRYB_NORMALNY;
        shm->st[i].pid = 0;
        shm->req_close[i] = 0;
    }
}

/** @brief Inicjalizuje IPC (kolejki + shm + sem). */
int serwis_ipc_init() {
    key_t kz  = key_make(65);
    key_t kzl = key_make(66);
    key_t kr  = key_make(67);
    key_t kk  = key_make(70);
    key_t ke  = key_make(71);
    key_t ksh = key_make(68);
    key_t kse = key_make(69);
    if (kz==-1 || kzl==-1 || kr==-1 || kk==-1 || ke==-1 || ksh==-1 || kse==-1) return SERWIS_IPC_ERR;

    q_zgl = msgget(kz, IPC_CREAT | PERM);
    if (q_zgl == -1) { perror("[IPC] msgget zgl"); return SERWIS_IPC_ERR; }

    q_zlec = msgget(kzl, IPC_CREAT | PERM);
    if (q_zlec == -1) { perror("[IPC] msgget zlec"); return SERWIS_IPC_ERR; }

    q_rap = msgget(kr, IPC_CREAT | PERM);
    if (q_rap == -1) { perror("[IPC] msgget rap"); return SERWIS_IPC_ERR; }

    q_kasa = msgget(kk, IPC_CREAT | PERM);
    if (q_kasa == -1) { perror("[IPC] msgget kasa"); return SERWIS_IPC_ERR; }

    q_ext = msgget(ke, IPC_CREAT | PERM);
    if (q_ext == -1) { perror("[IPC] msgget ext"); return SERWIS_IPC_ERR; }

    bool shm_created = false;
    shm_id = shmget(ksh, sizeof(SerwisStatystyki), IPC_CREAT | IPC_EXCL | PERM);
    if (shm_id == -1) {
        if (errno == EEXIST) {
            shm_id = shmget(ksh, sizeof(SerwisStatystyki), IPC_CREAT | PERM);
            if (shm_id == -1 && errno == EINVAL) {
                int old_id = shmget(ksh, 1, PERM);
                if (old_id != -1) shmctl(old_id, IPC_RMID, nullptr);
                shm_id = shmget(ksh, sizeof(SerwisStatystyki), IPC_CREAT | IPC_EXCL | PERM);
                if (shm_id != -1) shm_created = true;
            }
        }
    } else {
        shm_created = true;
    }
    if (shm_id == -1) { perror("[IPC] shmget"); return SERWIS_IPC_ERR; }

    void* p = shmat(shm_id, nullptr, 0);
    if (p == (void*)-1) { perror("[IPC] shmat"); return SERWIS_IPC_ERR; }
    shm = (SerwisStatystyki*)p;

    bool sem_created = false;
    sem_id = semget(kse, 1, IPC_CREAT | IPC_EXCL | PERM);
    if (sem_id == -1) {
        if (errno == EEXIST) sem_id = semget(kse, 1, IPC_CREAT | PERM);
    } else {
        sem_created = true;
    }
    if (sem_id == -1) { perror("[IPC] semget"); return SERWIS_IPC_ERR; }

    if (sem_created) {
        semun u{}; u.val = 1;
        if (semctl(sem_id, 0, SETVAL, u) == -1) perror("[IPC] semctl SETVAL");
    }

    if (shm_created) {
        shm_reset_all();
    } else {
        int r;
        while ((r = sem_lock()) == -2) {}
        if (r == 0) {
            if (shm->pozar) shm->pozar = 0;
            while ((r = sem_unlock()) == -2) {}
        }
    }

    return SERWIS_IPC_OK;
}

/** @brief Odlacza SHM. */
void serwis_ipc_detach() {
    if (shm) {
        shmdt((void*)shm);
        shm = nullptr;
    }
}

/** @brief Usuwa wszystkie zasoby IPC. */
void serwis_ipc_cleanup_all() {
    if (q_zgl  != -1) if (msgctl(q_zgl,  IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID zgl");
    if (q_zlec != -1) if (msgctl(q_zlec, IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID zlec");
    if (q_rap  != -1) if (msgctl(q_rap,  IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID rap");
    if (q_kasa != -1) if (msgctl(q_kasa, IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID kasa");
    if (q_ext  != -1) if (msgctl(q_ext,  IPC_RMID, nullptr) == -1) perror("[IPC] msgctl RMID ext");
    if (shm_id != -1) if (shmctl(shm_id, IPC_RMID, nullptr) == -1) perror("[IPC] shmctl RMID");
    if (sem_id != -1) if (semctl(sem_id, 0, IPC_RMID) == -1) perror("[IPC] semctl RMID");

    q_zgl = q_zlec = q_rap = q_kasa = q_ext = -1;
    shm_id = sem_id = -1;
}

int serwis_ipc_send_zgl(const Samochod& s) {
    MsgZgl m{}; m.mtype = 1; m.s = s;
    if (msgsnd(q_zgl, &m, sizeof(MsgZgl) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd zgl");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_zgl(Samochod& s) {
    MsgZgl m{};
    while (true) {
        ssize_t r = msgrcv(q_zgl, &m, sizeof(MsgZgl) - sizeof(long), 1, 0);
        if (r >= 0) { s = m.s; return SERWIS_IPC_OK; }
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv zgl");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_try_recv_zgl(Samochod& s) {
    MsgZgl m{};
    while (true) {
        ssize_t r = msgrcv(q_zgl, &m, sizeof(MsgZgl) - sizeof(long), 1, IPC_NOWAIT);
        if (r >= 0) { s = m.s; return SERWIS_IPC_OK; }
        if (errno == ENOMSG) return SERWIS_IPC_NO_MSG;
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv zgl (nowait)");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_send_zlec(const Zlecenie& z) {
    MsgZlec m{}; m.mtype = 100 + z.stanowisko_id; m.z = z;
    if (msgsnd(q_zlec, &m, sizeof(MsgZlec) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd zlec");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_zlec(int stanowisko_id, Zlecenie& z) {
    MsgZlec m{};
    long type = 100 + stanowisko_id;
    while (true) {
        ssize_t r = msgrcv(q_zlec, &m, sizeof(MsgZlec) - sizeof(long), type, 0);
        if (r >= 0) { z = m.z; return SERWIS_IPC_OK; }
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv zlec");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_send_rap(const Raport& r) {
    MsgRap m{}; m.mtype = 1; m.r = r;
    if (msgsnd(q_rap, &m, sizeof(MsgRap) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd rap");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_rap(Raport& r) {
    MsgRap m{};
    while (true) {
        ssize_t x = msgrcv(q_rap, &m, sizeof(MsgRap) - sizeof(long), 1, 0);
        if (x >= 0) { r = m.r; return SERWIS_IPC_OK; }
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv rap");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_try_recv_rap(Raport& r) {
    MsgRap m{};
    while (true) {
        ssize_t x = msgrcv(q_rap, &m, sizeof(MsgRap) - sizeof(long), 1, IPC_NOWAIT);
        if (x >= 0) { r = m.r; return SERWIS_IPC_OK; }
        if (errno == ENOMSG) return SERWIS_IPC_NO_MSG;
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv rap (nowait)");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_send_kasa(const Raport& r) {
    MsgRap m{}; m.mtype = 1; m.r = r;
    if (msgsnd(q_kasa, &m, sizeof(MsgRap) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd kasa");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_kasa(Raport& r) {
    MsgRap m{};
    while (true) {
        ssize_t x = msgrcv(q_kasa, &m, sizeof(MsgRap) - sizeof(long), 1, 0);
        if (x >= 0) { r = m.r; return SERWIS_IPC_OK; }
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv kasa");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_send_extra_req(const SerwisExtraReq& r) {
    MsgExtReq m{}; m.mtype = 1; m.r = r;
    if (msgsnd(q_ext, &m, sizeof(MsgExtReq) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd ext_req");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_extra_req(SerwisExtraReq& r) {
    MsgExtReq m{};
    while (true) {
        ssize_t x = msgrcv(q_ext, &m, sizeof(MsgExtReq) - sizeof(long), 1, 0);
        if (x >= 0) { r = m.r; return SERWIS_IPC_OK; }
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv ext_req");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_try_recv_extra_req(SerwisExtraReq& r) {
    MsgExtReq m{};
    while (true) {
        ssize_t x = msgrcv(q_ext, &m, sizeof(MsgExtReq) - sizeof(long), 1, IPC_NOWAIT);
        if (x >= 0) { r = m.r; return SERWIS_IPC_OK; }
        if (errno == ENOMSG) return SERWIS_IPC_NO_MSG;
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv ext_req (nowait)");
        return SERWIS_IPC_ERR;
    }
}

int serwis_ipc_send_extra_resp(const SerwisExtraResp& r) {
    MsgExtResp m{}; m.mtype = 1000 + r.id_klienta; m.r = r;
    if (msgsnd(q_ext, &m, sizeof(MsgExtResp) - sizeof(long), 0) == -1) {
        perror("[IPC] msgsnd ext_resp");
        return SERWIS_IPC_ERR;
    }
    return SERWIS_IPC_OK;
}

int serwis_ipc_recv_extra_resp(int id_klienta, SerwisExtraResp& r) {
    MsgExtResp m{};
    long type = 1000 + id_klienta;
    while (true) {
        ssize_t x = msgrcv(q_ext, &m, sizeof(MsgExtResp) - sizeof(long), type, 0);
        if (x >= 0) { r = m.r; return SERWIS_IPC_OK; }
        if (errno == EINTR) {
            if (shm && shm->pozar) return SERWIS_IPC_ERR;
            continue;
        }
        perror("[IPC] msgrcv ext_resp");
        return SERWIS_IPC_ERR;
    }
}

int serwis_stat_get(SerwisStatystyki& out) {
    if (!shm) return SERWIS_IPC_ERR;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return SERWIS_IPC_ERR;

    out = *shm;

    while ((r = sem_unlock()) == -2) {}
    if (r != 0) return SERWIS_IPC_ERR;

    return SERWIS_IPC_OK;
}

void serwis_set_pozar(int v) {
    if (!shm) return;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return;

    shm->pozar = v ? 1 : 0;

    while ((r = sem_unlock()) == -2) {}
    (void)r;
}

int serwis_get_pozar() {
    if (!shm) return 0;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return 0;

    int v = shm->pozar;

    while ((r = sem_unlock()) == -2) {}
    (void)r;

    return v;
}

void serwis_time_set(int minuty) {
    if (!shm) return;
    if (minuty < 0) minuty = 0;
    minuty %= 1440;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return;

    shm->sim_time_min = minuty;

    while ((r = sem_unlock()) == -2) {}
    (void)r;
}

int serwis_time_get() {
    if (!shm) return 0;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return 0;

    int v = shm->sim_time_min;

    while ((r = sem_unlock()) == -2) {}
    (void)r;

    if (v < 0) v = 0;
    v %= 1440;
    return v;
}

void serwis_station_set_busy(int id, int busy, char marka, int kryt, int dodatkowe, int tryb) {
    if (!shm || id < 1 || id > 8) return;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return;

    shm->st[id].zajete = busy ? 1 : 0;
    shm->st[id].marka = marka;
    shm->st[id].krytyczna = kryt ? 1 : 0;
    shm->st[id].dodatkowe = dodatkowe ? 1 : 0;
    shm->st[id].tryb = tryb;

    while ((r = sem_unlock()) == -2) {}
    (void)r;
}

void serwis_station_inc_done(int id) {
    if (!shm || id < 1 || id > 8) return;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return;

    shm->st[id].obsluzone++;

    while ((r = sem_unlock()) == -2) {}
    (void)r;
}

void serwis_station_set_closed(int id, int closed) {
    if (!shm || id < 1 || id > 8) return;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return;

    shm->st[id].zamkniete = closed ? 1 : 0;

    while ((r = sem_unlock()) == -2) {}
    (void)r;
}

void serwis_req_close(int id, int v) {
    if (!shm || id < 1 || id > 8) return;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return;

    shm->req_close[id] = v ? 1 : 0;

    while ((r = sem_unlock()) == -2) {}
    (void)r;
}

int serwis_get_req_close(int id) {
    if (!shm || id < 1 || id > 8) return 0;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return 0;

    int v = shm->req_close[id];

    while ((r = sem_unlock()) == -2) {}
    (void)r;

    return v;
}

void serwis_station_set_pid(int id, int pid) {
    if (!shm || id < 1 || id > 8) return;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return;

    shm->st[id].pid = pid;

    while ((r = sem_unlock()) == -2) {}
    (void)r;
}

int serwis_station_get_pid(int id) {
    if (!shm || id < 1 || id > 8) return 0;

    int r;
    while ((r = sem_lock()) == -2) {}
    if (r != 0) return 0;

    int v = shm->st[id].pid;

    while ((r = sem_unlock()) == -2) {}
    (void)r;

    return v;
}
