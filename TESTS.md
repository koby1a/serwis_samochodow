# Scenariusze testowe T1–T5

Poniżej masz krótką checklistę uruchomienia i weryfikacji testów z pliku `KOBYLARCZYK_MATEUSZ_155279_opis_SERWIS_SAMOCHODOW.md`.

## Wspólne kroki

1) Zbuduj projekt:
```
cmake --build cmake-build-debug -j
```

2) Uruchom symulację z konfiguracją:
```
./serwis_samochodow --config ../configs/TX.cfg
```

3) Sprawdź logi w `raport_symulacji.log` oraz dashboard.

## T1 – 2000 samochodów tylko marki A

Start:
```
./serwis_samochodow --config ../configs/T1.cfg
```

Weryfikacja:
- brak odrzuceń z powodu nieobsługiwanej marki,
- brak przydziału na stanowisko 8 (bo tylko A),
- rozkład na stanowiska 1–7.

## T2 – Kolejki i dynamiczne otwieranie okienek (burst)

Start:
```
./serwis_samochodow --config ../configs/T2.cfg
```

Weryfikacja:
- w logach pojawiają się wpisy `okienko_open` oraz `okienko_close`,
- po burstach 4 i 10 kierowców następuje otwarcie 2. i 3. okienka,
- po spadku kolejki zamknięcie okienek zgodnie z progami (≤2 i ≤3).

## T3 – Bramka czasu + usterki krytyczne (deterministyczne)

Start:
```
./serwis_samochodow --config ../configs/T3.cfg
```

Weryfikacja:
- klient z czasem 7:30 (do otwarcia = 30) jest odrzucony,
- klient z czasem 7:40 (do otwarcia = 20) jest przyjęty,
- klient krytyczny przed otwarciem jest przyjęty,
- klient po zamknięciu bez krytycznej jest odrzucony,
- klient po zamknięciu z krytyczną jest przyjęty.

## T4 – Sygnały 1–3 (kierownik)

Start:
```
./serwis_samochodow --config ../configs/T4.cfg
```

W trakcie działania w oknie `kierownik`:
- wybierz 2 (przyspiesz) dla wybranego stanowiska,
- ponowne 2 powinno być ignorowane (jeśli już tryb przyspieszony),
- wybierz 3 (przywróć) dla tego samego stanowiska,
- wybierz 1 (zamknij) – zamknięcie po zakończeniu bieżącej naprawy.

Weryfikacja:
- krótsze czasy po sygnale 2,
- powrót do normalnych czasów po sygnale 3,
- brak nowych zleceń po zamknięciu.

## T5 – Pożar (sygnał 4)

Start:
```
./serwis_samochodow --config ../configs/T5.cfg
```

W trakcie działania w oknie `kierownik`:
- wybierz 4 (pożar)

Weryfikacja:
- log o pożarze i zakończeniu pracy,
- procesy kończą działanie,
- zasoby IPC sprzątane po zakończeniu.
