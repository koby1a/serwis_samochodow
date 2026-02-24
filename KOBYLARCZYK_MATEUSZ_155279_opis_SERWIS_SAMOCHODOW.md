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

## 7. Scenariusze testowe (min. 4, zrealizowane 4)

Poniżej opis testów zrealizowanych i opisanych w raporcie.

---

### Test T1 – 5000 samochodów tylko marki A (start synchroniczny)

**Cel:**  
Sprawdzenie wydajności systemu przy dużej liczbie zgłoszeń oraz zachowania kolejek przy jednoczesnym starcie wielu procesów‑kierowców.

**Wejście:**

- Serwis otwarty.
- Generacja **5000** samochodów wyłącznie marki **A** (scenariusz `T1`).
- Start synchroniczny (wspólne zwolnienie procesów).

**Wynik (z raportu):**

- brak odrzuceń z powodu nieobsługiwanej marki,
- brak odrzuceń z tytułu godzin pracy,
- widoczny synchroniczny start w logach.

---

### Test T2 – 5000 krytycznych aut poza godzinami

**Cel:**  
Sprawdzenie reguły „poza godzinami przyjmujemy tylko krytyczne usterki” oraz filtrowania marek do obsługiwanych.

**Wejście:**

- Serwis poza godzinami pracy.
- Generacja **5000** samochodów krytycznych (scenariusz `T2`).

**Wynik (z raportu):**

- brak odrzuceń z powodu godzin pracy (wszystkie auta krytyczne),
- brak odrzuceń z powodu nieobsługiwanej marki,
- stabilna obsługa dużej liczby zgłoszeń.

---

### Test T3 – Obciążenie raportów/kasy (IPC)

**Cel:**  
Przetestowanie, czy kolejki `rap` i `kasa` nie zapychają się przy maksymalnym przepływie raportów.

**Wejście:**

- Scenariusz `T3` z minimalnym czasem napraw (maksymalny przepływ raportów).

**Wynik (z raportu):**

- szybki napływ raportów,
- brak blokad i stabilna obsługa dużej liczby raportów.

---

### Test T4 – Dodatkowe usterki (100% akceptacji)

**Cel:**  
Przetestowanie komunikacji `ext_req`/`ext_resp` oraz wpływu zawsze‑akceptowanych rozszerzeń na czas/koszt napraw.

**Wejście:**

- Scenariusz `T4` z wymuszoną akceptacją dodatkowych usterek.

**Wynik (z raportu):**

- wszystkie dodatkowe usterki są akceptowane,
- brak zacięć w kanałach `ext_req`/`ext_resp`,
- stabilna obsługa przy dużej liczbie zgłoszeń.

---

## 8. Logi i raport z przebiegu

Przykładowy podział logów:

- `log_serwis.txt` – globalne zdarzenia (przyjazdy, zmiany w kolejkach, otwieranie/zamykanie stanowisk, sygnały, pożar),
- `log_stanowisko_X.txt` – szczegóły pracy stanowiska X (ID pojazdu, marka, czasy rozpoczęcia/zakończenia, tryb normalny/przyspieszony),
- `log_platnosci.txt` – informacje o płatnościach (ID pojazdu, lista usług, kwota, status).

Na podstawie tych logów będzie można zweryfikować poprawność działania oraz przeprowadzenie testów T1–T5.

## 9. Linki do kodu

Linki do fragmentów kodu w repozytorium GitHub, obrazujących użycie wymaganych konstrukcji:

- tworzenie procesów: `fork()` ([main.cpp#L39](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L39)), `execv()` ([main.cpp#L45](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L45)), `waitpid()` ([main.cpp#L213](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L213)), `exit()` ([tests/test_model.cpp#L5](https://github.com/koby1a/serwis_samochodow/blob/master/tests/test_model.cpp#L5)),
- mechanizmy synchronizacji (semafory System V): `semget()` ([serwis_ipc.cpp#L252](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L252)), `semop()` ([serwis_ipc.cpp#L142](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L142)), `semctl()` ([serwis_ipc.cpp#L262](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L262)),
- komunikacja międzyprocesowa: `msgget()` ([serwis_ipc.cpp#L212](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L212)), `msgsnd()` ([serwis_ipc.cpp#L331](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L331)), `msgrcv()` ([serwis_ipc.cpp#L351](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L351)), `shmget()` ([serwis_ipc.cpp#L231](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L231)), `shmat()` ([serwis_ipc.cpp#L247](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L247)), `shmdt()` ([serwis_ipc.cpp#L287](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L287)), `shmctl()` ([serwis_ipc.cpp#L300](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L300)),
- obsługa sygnałów: `sigaction()` ([main.cpp#L126](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L126)), `sigaction()` mechanik ([mechanik.cpp#L27](https://github.com/koby1a/serwis_samochodow/blob/master/mechanik.cpp#L27)),
- obsługa błędów: `perror()` ([serwis_ipc.cpp#L55](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L55)), `errno` ([serwis_ipc.cpp#L143](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L143)).
