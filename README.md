# Raport – Symulacja serwisu samochodowego

## 1. Cel projektu
Celem projektu było zbudowanie wieloprocesowej symulacji serwisu samochodowego, zgodnej z wymaganiami przedmiotu. Symulacja odwzorowuje obsługę klientów i proces napraw w warunkach ograniczonych zasobów (okienka obsługi, stanowiska mechaników, kasa), z uwzględnieniem godzin pracy, kolejek, usterek krytycznych, sygnałów sterujących oraz komunikacji międzyprocesowej (IPC).

## 2. Środowisko i narzędzia
System operacyjny: Windows 11 Pro 25H2 + WSL (Ubuntu 22.04.5 LTS)
Język: C++
Edytor: CLion 2025.3
Mechanizmy IPC: kolejki komunikatów System V, pamięć współdzielona System V, semafory System V
Sterowanie asynchroniczne: sygnały POSIX
Link do projektu na GitHubie:
[github.com/koby1a/serwis_samochodow](https://github.com/koby1a/serwis_samochodow)

## 3. Budowanie i uruchomienie
Budowanie:
```bash
cmake --build cmake-build-debug -j
```
Uruchomienie symulacji:
```bash
./serwis_samochodow
```
Ogólne uruchamianie testów:
```bash
./serwis_samochodow --config ../configs/TX.cfg
```
Gdzie `X` to numer testu (1–4).

## 4. Pokrycie wymagań (skrót)
Wieloprocesowość: osobne procesy dla głównych ról (generator zgłoszeń, obsługa klienta, mechanicy, kasa, kierownik, dashboard).
IPC: wymiana danych wyłącznie przez formalne mechanizmy (kolejki, SHM, semafory).
Synchronizacja SHM: dostęp do współdzielonego stanu chroniony semaforem (mutex).
Sterowanie sygnałami: tryby pracy mechaników oraz zdarzenia globalne (np. zamknięcie stanowiska, przyspieszenie/normalizacja, alarm).
Godziny pracy: reguły przyjmowania zgłoszeń zależne od Tp..Tk oraz krytyczności usterki (z progiem T1).
Dynamiczna obsługa klienta: 1–3 okienka aktywowane na podstawie progów obciążenia K1/K2.
Ograniczenia stanowisk: stanowiska 1–7 obsługują marki A/E/I/O/U/Y, stanowisko 8 tylko U/Y.
Dodatkowe usterki: mechanik może zgłosić usterkę dodatkową (~20% przypadków), klient akceptuje ją z określonym prawdopodobieństwem (~80%).
Odporność na zapychanie IPC: rezerwacja wolnego miejsca w kolejkach (kontrola pojemności) oraz kontrolowane wygaszanie procesów komunikatami shutdown.

## 5. Architektura systemu
System składa się z niezależnych procesów uruchamianych przez proces główny:
main – inicjalizuje zasoby IPC, uruchamia pozostałe procesy, steruje zegarem symulacji i sprząta zasoby po zakończeniu.
kierowca (generator) – tworzy procesy zgodnie z parametrami i scenariuszem testowym.
pracownik_serwisu – proces nadrzędny uruchamiający do 3 okienek obsługi klienta; aktywuje/dezaktywuje je w zależności od obciążenia (K1/K2).
okienko_obsługi (1–3) – rejestruje klienta, weryfikuje warunki przyjęcia i zleca naprawę.
mechanik (8 procesów) – realizuje naprawy; w części przypadków zgłasza dodatkowe usterki i obsługuje sygnały sterujące od kierownika.
kasjer – finalizuje płatność po zakończeniu naprawy (zamknięcie zlecenia).
kierownik – nadaje sygnały do mechaników (zamknięcia, tryb przyspieszony/normalny, alarm).
dashboard – podgląd stanu serwisu (kolejki, stanowiska, czas symulacji) w czasie rzeczywistym.

Podział na procesy ogranicza współdzielenie stanu do minimum: logika biznesowa jest rozproszona, a stan globalny utrzymywany w pamięci współdzielonej z kontrolą dostępu semaforem.

## 6. Komunikacja i synchronizacja
### 6.1 Mechanizmy IPC
Kolejki komunikatów System V – przekazywanie zgłoszeń klientów, zleceń napraw, raportów, płatności oraz komunikacji dot. dodatkowych usterek.
Pamięć współdzielona (SHM) – bieżący stan serwisu (stanowiska, wskaźniki kolejek, czas symulacji, flagi zamknięć).
Semafor System V – mutex chroniący sekcje krytyczne związane z odczytem/zapisem SHM.

### 6.2 Sygnały (sterowanie)
Tryb pracy mechaników: przełączenie między trybem normalnym i przyspieszonym (skrót czasu napraw).
Zdarzenia awaryjne/organizacyjne: zamknięcie stanowiska, alarm (np. „pożar”) i inne sygnały opisane w projekcie.
Zasada: sygnały tylko sterują zachowaniem; dane operacyjne nadal płyną przez kolejki/SHM.

### 6.3 Ochrona przed zapchaniem kolejek i kontrolowane zakończenie
Przed wysłaniem komunikatu nadawca sprawdza dostępne miejsce w kolejce (np. przez `msgctl` i kontrolę limitów), aby uniknąć niekontrolowanych blokad.
Zakończenie pracy odbywa się komunikatami `shutdown` rozsyłanymi do procesów czytających kolejki; procesy kończą się kontrolowanie i zwalniają zasoby.
Proces `main` odpowiada za finalne sprzątanie IPC (kolejki, semafory, SHM) po zebraniu dzieci (`wait`/`waitpid`).

## 7. Jak to działa (opis mechaniki)
### 7.1 Przyjmowanie klienta i reguły godzin pracy
Obsługiwane marki: A, E, I, O, U, Y. Inne zgłoszenia są odrzucane na etapie rejestracji.
Okno czasowe: standardowo przyjmowani są klienci tylko w Tp..Tk.
Poza Tp..Tk: klient może zostać dopuszczony tylko, jeśli usterka jest krytyczna lub do otwarcia pozostało mniej niż T1.

### 7.2 Kierowanie na stanowiska i naprawa
Stanowiska 1–7 obsługują pełny zestaw marek A/E/I/O/U/Y.
Stanowisko 8 obsługuje wyłącznie marki U/Y (wymusza to specjalizację zasobu).
Po przyjęciu zgłoszenia okienko tworzy zlecenie i przesyła je do odpowiedniego mechanika/kolejki.

### 7.3 Dodatkowe usterki (ext_req/ext_resp)
W ok. 20% napraw mechanik identyfikuje dodatkową usterkę i wysyła propozycję (`ext_req`).
Klient podejmuje decyzję o akceptacji (ok. 80%) lub odrzuceniu; odpowiedź wraca kanałem `ext_resp`.
Akceptacja wpływa na czas i/lub koszt naprawy; odrzucenie powoduje powrót do podstawowego zlecenia.

### 7.4 Dynamiczne okienka obsługi (K1/K2)
Proces `pracownik_serwisu` uruchamia od 1 do 3 okienek.
Gdy obciążenie (np. długość kolejki zgłoszeń) przekroczy K1 – aktywowane jest 2. okienko; powyżej K2 – 3. okienko.
Przy spadku obciążenia okienka mogą zostać wygaszone, aby ograniczać koszty procesu.

### 7.5 Globalne przyspieszenie symulacji
Parametr `time_scale` działa jako globalny mnożnik (skalowanie sleep/opóźnień), umożliwiając przyspieszanie lub spowalnianie całej symulacji.
Tryb przyspieszony może być też uruchamiany sygnałem kierownika dla mechaników (lokalne skrócenie czasu napraw).

## 8. Logi i raport
Procesy logują kluczowe zdarzenia: przyjęcie/odrzucenie zgłoszenia, przydział stanowiska, start/koniec naprawy, płatność, usterki dodatkowe, sygnały sterujące.
Zalecane jest utrzymywanie spójnego formatu logów (czas symulacji + pid/proces + typ zdarzenia + identyfikator zlecenia).
Dashboard prezentuje stan serwisu w czasie rzeczywistym na podstawie SHM (liczniki i statusy).

## 9. Testy i scenariusze (wyniki)
Uwaga: program nie kończy symulacji automatycznie; po osiągnięciu stanu bez kolejek testy zakończono sygnałem SIGINT (lub równoważnym mechanizmem przerwania).

### 9.1 Scenariusze obciążeniowe (T1–T4)
T1 – 5000 samochodów tylko marki A.
Cel: sprawdzić wydajność przy dużej liczbie zgłoszeń i synchronicznym starcie generatora.
Wnioski: brak odrzuceń ofert (akceptacja wymuszona), brak odrzuceń z tytułu godzin pracy, widoczny skok kolejek przy starcie.

T2 – 5000 krytycznych aut poza godzinami.
Cel: zweryfikować regułę dopuszczania wyłącznie krytycznych usterek poza Tp..Tk oraz filtrowanie marek.
Wnioski: brak odrzuceń z powodu godzin pracy, brak odrzuceń z powodu nieobsługiwanej marki, stabilna obsługa obciążenia.

T3 – Obciążenie raportów/kasy (IPC).
Cel: sprawdzić, czy kanały raportów i płatności nie zapychają się przy maksymalnym przepływie.
Wnioski: szybki napływ raportów (minimalne czasy napraw), brak blokad, stabilna obsługa dużej liczby komunikatów.

T4 – Dodatkowe usterki (100%).
Cel: przetestować komunikację `ext_req`/`ext_resp` oraz wpływ zawsze-akceptowanych rozszerzeń na czas/koszt napraw.
Wnioski z logów: wszystkie dodatkowe usterki są akceptowane, brak zacięć w kanałach `ext_req`/`ext_resp`, stabilna obsługa przy dużej liczbie zgłoszeń.

## 10. Funkcje wymagane przez projekt (gdzie szukać)
Linki do fragmentów kodu w repozytorium GitHub, obrazujących użycie wymaganych konstrukcji:
- Tworzenie procesów:
- fork()  ([main.cpp#L41](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L41))
- execv() ([main.cpp#L47](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L47))
- waitpid() ([main.cpp#L215](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L215))
- exit() ([tests/test_model.cpp#L5](https://github.com/koby1a/serwis_samochodow/blob/master/tests/test_model.cpp#L5))

- Mechanizmy synchronizacji (semafory System V):
- semget() ([serwis_ipc.cpp#L239](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L239))
- semop()  ([serwis_ipc.cpp#L129](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L129))
- semctl() ([serwis_ipc.cpp#L249](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L249))

- Komunikacja międzyprocesowa:
- msgget() ([serwis_ipc.cpp#L199](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L199))
- msgsnd() ([serwis_ipc.cpp#L318](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L318))
- msgrcv() ([serwis_ipc.cpp#L338](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L338))
- shmget() ([serwis_ipc.cpp#L218](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L218))
- shmat()  ([serwis_ipc.cpp#L234](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L234))
- shmdt()  ([serwis_ipc.cpp#L274](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L274))
- shmctl() ([serwis_ipc.cpp#L287](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L287))

- Obsługa sygnałów:
- sigaction() ([main.cpp#L128](https://github.com/koby1a/serwis_samochodow/blob/master/main.cpp#L128))
- sigaction() mechanik ([mechanik.cpp#L27](https://github.com/koby1a/serwis_samochodow/blob/master/mechanik.cpp#L27))

- Obsługa błędów:
- perror() ([serwis_ipc.cpp#L56](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L56))
- errno ([serwis_ipc.cpp#L55](https://github.com/koby1a/serwis_samochodow/blob/master/serwis_ipc.cpp#L55))

## 11. Co zostało zmienione
Podczas testów zaimplementowano odrzucenie ofert (ok. 2% kierowców nie akceptuje warunków i odjeżdża z serwisu bez naprawy). Wynikiem takiej operacji przy uruchomieniu testu i generowaniu 5000 samochodów było odrzucanie przez kierowców ofert naprawy. Dlatego też wynik poniższej komendy dawał nam liczbę wygenerowanych kierowców w liczbie 5002:
```bash
ps aux | grep 'kierowca' | wc -l
```
Natomiast po zliczeniu płatności za usługi poniższą komendą:
```bash
grep -o 'platnosc' raport_symulacji.log | wc -l
```
Wynikiem było 4910 płatności.

Po zastosowanej zmianie i wyeliminowaniu 2% odrzuceń przez kierowców końcowy wynik komendy:
```bash
grep -o 'platnosc' raport_symulacji.log | wc -l
```
Był poprawny. Było to dokładnie 5000 płatności.

Link do zmienionego fragmentu:
[pracownik_serwisu.cpp#L203](https://github.com/koby1a/serwis_samochodow/blob/master/pracownik_serwisu.cpp#L203)
