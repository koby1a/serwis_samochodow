# Serwis samochodów – opis projektu

## 1. Dane autora

- Imię i nazwisko: **Mateusz Kobylarczyk**
- Nr albumu: **155279**
- Grupa laboratoryjna: **LAB_03**
- Prowadzący: **dr inż. Anna Jasińska-Suwada oraz mgr inż. Jan Wojtas**

Repozytorium GitHub (publiczne):  
`https://github.com/koby1a/serwis_samochodow`

Projekt będzie zaimplementowany w języku **C/C++**, zgodnie z wymaganiami przedmiotu.

## 2. Opis ogólny systemu

Symulowany jest **serwis samochodowy** działający w godzinach od **Tp** do **Tk**.  
Serwis:

- obsługuje tylko samochody marek: **A, E, I, O, U, Y** (6 marek),
- **nie obsługuje** pozostałych marek z zakresu **A–Z**,
- posiada **8 stanowisk napraw**:
    - stanowiska **1–7** – naprawa marek **A, E, I, O, U, Y**,
    - stanowisko **8** – naprawa tylko marek **U i Y**.

Samochody (marki A–Z) pojawiają się w serwisie w **losowych chwilach**, również **poza godzinami pracy**.  
Dla obsługiwanych marek realizowana jest ścieżka: rejestracja → diagnoza → naprawa → płatność → wyjazd.  
Raport z przebiegu symulacji zapisywany jest do **plików tekstowych** (logi).

## 3. Role/procesy w systemie

W projekcie zostaną zaimplementowane procesy (ewentualnie z wątkami), odpowiadające następującym rolom:

1. **Kierownik serwisu**
    - wysyła sygnały do mechaników:
        - `sygnał1` – zamknięcie wybranego stanowiska po zakończeniu bieżącej naprawy,
        - `sygnał2` – przyspieszenie napraw na stanowisku o 50%,
        - `sygnał3` – przywrócenie normalnego czasu napraw na stanowisku,
        - `sygnał4` – pożar, zamknięcie całego serwisu i przerwanie pracy.
    - monitoruje obciążenie serwisu.

2. **Pracownik serwisu (obsługa klienta, 1–3 stanowiska)**
    - rejestruje przyjazd kierowców,
    - sprawdza, czy marka pojazdu jest obsługiwana,
    - ustala:
        - szacowany **czas naprawy**,
        - przewidywany **koszt** na podstawie **cennika (≥ 30 usług)**,
    - obsługuje sytuacje:
        - odrzucenia warunków przez klienta (ok. **2%** klientów),
        - dodatkowych usterek zgłaszanych przez mechanika (20% przypadków) oraz decyzję klienta o rozszerzeniu zakresu napraw (ok. 80% zgadza się),
    - kieruje pojazdy na wolne stanowiska napraw.

3. **Mechanik / stanowisko naprawcze (8 stanowisk)**
    - obsługuje pojazdy zgodnie z zasadami:
        - stanowiska 1–7: marki A/E/I/O/U/Y,
        - stanowisko 8: tylko marki U/Y,
    - realizuje naprawę w przydzielonym czasie (modyfikowanym przez sygnały 2/3),
    - z prawdopodobieństwem ok. **20%** wykrywa dodatkowe usterki i zgłasza je do pracownika serwisu,
    - po zakończeniu naprawy przekazuje raport o wykonanych pracach.

4. **Kasjer**
    - na podstawie raportu z napraw wylicza ostateczną kwotę,
    - przyjmuje płatność,
    - po opłaceniu informuje o możliwości odbioru kluczyków.

5. **Kierowca**
    - przyjeżdża do serwisu o losowej godzinie (także poza Tp–Tk),
    - jeśli serwis jest zamknięty:
        - może czekać w kolejce, **jeżeli** usterka jest **krytyczna** (zdefiniowane ≥3 typy) **lub** czas do otwarcia jest krótszy niż **T1**,
    - akceptuje lub odrzuca warunki naprawy (ok. 2% odrzuca),
    - w razie wykrycia dodatkowych usterek decyduje o rozszerzeniu zakresu napraw (ok. 80% akceptuje),
    - po naprawie płaci, odbiera kluczyki i opuszcza serwis.

## 4. Kolejki i stanowiska obsługi klienta

W serwisie działają **3 stanowiska obsługi klienta**, z dynamicznym uruchamianiem:

- zawsze działa przynajmniej **1 stanowisko**,
- jeśli **w kolejce do rejestracji**:
    - stoi więcej niż **K1** kierowców (**K1 ≥ 3**) → otwiera się **drugie** stanowisko,
    - liczba klientów spadnie do **≤ 2** → drugie stanowisko jest zamykane,
- jeśli w kolejce stoi więcej niż **K2** kierowców (**K2 ≥ 5**) → otwiera się **trzecie** stanowisko,
    - liczba klientów spadnie do **≤ 3** → trzecie stanowisko jest zamykane.

Zasady te będą uwzględnione w logice obsługi kolejki i w testach.

## 5. Parametry symulacji i dane

Parametry konfigurowalne (np. przez plik konfiguracyjny lub argumenty programu):

- godziny pracy serwisu: **Tp**, **Tk**,
- próg czasu oczekiwania przed otwarciem: **T1**,
- progi **K1**, **K2** (np. domyślnie `K1 = 3`, `K2 = 5`),
- czas trwania symulacji,
- intensywność napływu klientów (średnia liczba kierowców na jednostkę czasu),
- rozkład czasu napraw (min–max),
- cennik co najmniej **30 usług**.

W testach możliwe będzie ustawienie stałego **ziarna generatora losowego**, aby wyniki były powtarzalne.

## 6. Planowane mechanizmy systemowe

Projekt będzie wykorzystywał co najmniej 4 z wymaganych konstrukcji, planowane:

1. **Procesy**:
    - `fork()`, `exec()`, `wait()`, `exit()` – osobne procesy dla kierownika, mechaników, pracowników obsługi, kasjera.

2. **Synchronizacja procesów/wątków**:
    - semafory System V / POSIX: `semget()`, `semop()`, `semctl()` – do obsługi kolejek i stanowisk,
    - ewentualnie mutexy/zmienne warunkowe dla wątków (`pthread_*`).

3. **Komunikacja międzyprocesowa (co najmniej dwa mechanizmy)**:
    - **kolejki komunikatów** – do przekazywania zdarzeń (np. zgłoszenie dodatkowej usterki, przekazanie pojazdu na stanowisko),
    - **pamięć dzielona** – do przechowywania wspólnego stanu systemu (listy pojazdów, kolejek).

4. **Obsługa sygnałów**:
    - `sigaction()` / `signal()` – obsługa sygnałów 1–4 (zamknięcie stanowiska, przyspieszenie/zwolnienie, pożar).

5. **Pliki i logi**:
    - `open()`, `write()`, `read()`, `close()`, `creat()`, `unlink()` – zapis raportów z przebiegu symulacji.

Dodatkowo:

- wszystkie dane wprowadzane przez użytkownika będą **walidowane** (np. zakresy parametrów, liczby procesów/stanowisk),
- tworzonym strukturom IPC (pamięć dzielona, semafory, kolejki komunikatów) zostaną nadane **minimalne wymagane prawa dostępu**,
- po zakończeniu symulacji wszystkie struktury IPC zostaną **poprawnie usunięte**.

Wszystkie funkcje systemowe będą miały obsługę błędów przez `perror()` i `errno`.

## 7. Scenariusze testowe (min. 4, planowane 5)

Poniżej opis testów, które będą zrealizowane i opisane w raporcie.

---

### Test T1 – 2000 samochodów tylko marki A

**Cel:**  
Sprawdzenie dużej liczby klientów jednej marki oraz poprawnego przydziału na stanowiska 1–7.

**Wejście:**

- Serwis otwarty.
- Generacja **2000** samochodów wyłącznie marki **A** (scenariusz `A_ONLY`).

**Oczekiwany wynik:**

- brak odrzuceń z powodu nieobsługiwanej marki,
- brak przydziału na stanowisko 8,
- rozkład napraw na stanowiskach 1–7,
- brak zakleszczeń, poprawne zakończenie symulacji.

---

### Test T2 – Kolejki i dynamiczne otwieranie okienek (burst)

**Cel:**  
Sprawdzenie progów **K1** i **K2** przy otwieraniu/zamykaniu okienek obsługi.

**Wejście:**

- Serwis otwarty, parametry: `K1 = 3`, `K2 = 5`.
- Seria „burstów” przyjazdów: **4**, **10**, **2**, **7**, **1**, **6** (scenariusz `BURST_K1K2`).

**Oczekiwany wynik:**

- po przekroczeniu **K1** otwiera się 2. okienko; zamyka się, gdy kolejka ≤2,
- po przekroczeniu **K2** otwiera się 3. okienko; zamyka się, gdy kolejka ≤3,
- zawsze działa co najmniej jedno okienko,
- zdarzenia otwarcia/zamknięcia są zapisane w logu (`okienko_open` / `okienko_close`).

---

### Test T3 – Bramka czasu + usterki krytyczne (deterministyczne)

**Cel:**  
Sprawdzenie warunków dopuszczenia klientów przed/po godzinach pracy w sposób deterministyczny.

**Wejście:**

- Godziny pracy: `Tp = 8:00`, `Tk = 16:00`,
- `T1 = 30` minut,
- scenariusz `TIME_GATES` z ustalonymi czasami przyjazdu:
    - 7:30 (do otwarcia = 30) – niekrytyczny,
    - 7:40 (do otwarcia = 20) – niekrytyczny,
    - 6:40 – krytyczny,
    - 16:10 – niekrytyczny,
    - 16:30 – krytyczny.

**Oczekiwany wynik:**

- 7:30 niekrytyczny → odrzucony (do otwarcia = T1),
- 7:40 niekrytyczny → przyjęty (do otwarcia < T1),
- krytyczny przed otwarciem → przyjęty,
- po zamknięciu niekrytyczny → odrzucony,
- po zamknięciu krytyczny → przyjęty,
- zachowanie odnotowane w logach.

---

### Test T4 – Zamknięcie stanowiska i zmiana czasu naprawy (sygnały 1–3)

**Cel:**  
Sprawdzenie reakcji mechaników na sygnały:

- `sygnał1` – zamknięcie stanowiska po bieżącej naprawie,
- `sygnał2` – przyspieszenie napraw o 50%,
- `sygnał3` – przywrócenie normalnego czasu.

**Wejście:**

- kilka pojazdów w kolejce na dane stanowisko,
- typowe czasy napraw (np. 10–20 jednostek czasu).

**Oczekiwany wynik:**

- po `sygnał2` naprawy na danym stanowisku trwają ~50% krócej,
- kolejne `sygnał2` są ignorowane, jeśli stanowisko już pracuje w trybie przyspieszonym,
- `sygnał3` działa tylko po wcześniejszym `sygnał2` i przywraca normalne czasy,
- po `sygnał1`:
    - bieżąca naprawa jest dokańczana,
    - po jej zakończeniu stanowisko nie przyjmuje nowych pojazdów,
    - nowe pojazdy kierowane są na inne stanowiska.

---

### Test T5 – Pożar (sygnał4) i awaryjne zatrzymanie serwisu

**Cel:**  
Sprawdzenie awaryjnego zatrzymania całej symulacji po `sygnał4` (pożar).

**Wejście:**

- Serwis w trakcie intensywnej pracy:
    - kilka pojazdów w naprawie,
    - klienci w kolejce,
    - trwające obsługi/diagnozy/płatności.

**Oczekiwany wynik:**

- po `sygnał4`:
    - wszystkie procesy (mechanicy, pracownicy, kasjer, kierowcy) kończą działanie,
    - trwające naprawy są przerywane (nie kończą się normalnie),
    - nie są przyjmowani nowi klienci,
    - wszystkie zasoby IPC (pamięć dzielona, semafory, kolejki) są zwalniane,
- w logach wyraźny wpis o pożarze i zamknięciu serwisu.

---

## 8. Logi i raport z przebiegu

Przykładowy podział logów:

- `log_serwis.txt` – globalne zdarzenia (przyjazdy, zmiany w kolejkach, otwieranie/zamykanie stanowisk, sygnały, pożar),
- `log_stanowisko_X.txt` – szczegóły pracy stanowiska X (ID pojazdu, marka, czasy rozpoczęcia/zakończenia, tryb normalny/przyspieszony),
- `log_platnosci.txt` – informacje o płatnościach (ID pojazdu, lista usług, kwota, status).

Na podstawie tych logów będzie można zweryfikować poprawność działania oraz przeprowadzenie testów T1–T5.

## 9. Linki do kodu

Linki do fragmentów kodu w repozytorium GitHub, obrazujących użycie wymaganych konstrukcji:

- tworzenie procesów: `fork()` ([main.cpp#L25](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L25)), `execv()` ([main.cpp#L31](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L31)), `waitpid()` ([main.cpp#L190](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L190)), `exit()` ([tests/test_model.cpp#L5](https://github.com/koby1a/serwis_samochodow/blob/master/tests/test_model.cpp#L5)),
- mechanizmy synchronizacji (semafory System V): `semget()` ([serwis_ipc.cpp#L255](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L255)), `semop()` ([serwis_ipc.cpp#L135](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L135)), `semctl()` ([serwis_ipc.cpp#L265](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L265)),
- komunikacja międzyprocesowa: `msgget()` ([serwis_ipc.cpp#L215](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L215)), `msgsnd()` ([serwis_ipc.cpp#L314](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L314)), `msgrcv()` ([serwis_ipc.cpp#L334](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L334)), `shmget()` ([serwis_ipc.cpp#L234](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L234)), `shmat()` ([serwis_ipc.cpp#L250](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L250)), `shmdt()` ([serwis_ipc.cpp#L290](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L290)), `shmctl()` ([serwis_ipc.cpp#L303](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L303)),
- obsługa sygnałów: `sigaction()` ([main.cpp#L107](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L107)), `sigaction()` mechanik ([mechanik.cpp#L23](https://github.com/koby1a/serwis_samochodow/blob/master/mechanik.cpp#L23)),
- obsługa błędów: `perror()` ([serwis_ipc.cpp#L67](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L67)), `errno` ([serwis_ipc.cpp#L136](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L136)).
