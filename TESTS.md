# Scenariusze testowe T1–T5

Poniżej masz krótką checklistę uruchomienia i weryfikacji testów z pliku `KOBYLARCZYK_MATEUSZ_155279_opis_SERWIS_SAMOCHODOW.md`.

## Wspólne kroki

1) Zbuduj projekt:
```
cmake --build cmake-build-debug -j
```

2) Uruchom symulację z konfiguracją:
```
./serwis_samochodow --config configs/TX.cfg
```

3) Sprawdź logi w `raport_symulacji.log` oraz dashboard.

## T1 – Obsługa marek i przydział na stanowiska

Start:
```
./serwis_samochodow --config configs/T1.cfg
```

Weryfikacja:
- logi odrzuceń marek spoza A/E/I/O/U/Y,
- przydział marek U/Y do stanowiska 8,
- brak obsługi innych marek na stanowisku 8.

## T2 – Kolejki i dynamiczne otwieranie okienek

Start:
```
./serwis_samochodow --config configs/T2.cfg
```

Weryfikacja:
- w logach zmiany liczby okienek przy K1/K2,
- brak zejścia poniżej 1 aktywnego okienka.

## T3 – Klienci przed godzinami otwarcia i usterki krytyczne

Start:
```
./serwis_samochodow --config configs/T3.cfg
```

Weryfikacja:
- odrzucenia klientów przed Tp, gdy nie są krytyczni i do otwarcia >= T1,
- przyjęcia krytycznych oraz tych z czasem do otwarcia < T1.

## T4 – Sygnały 1–3 (kierownik)

Start:
```
./serwis_samochodow --config configs/T4.cfg
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
./serwis_samochodow --config configs/T5.cfg
```

W trakcie działania w oknie `kierownik`:
- wybierz 4 (pożar)

Weryfikacja:
- log o pożarze i zakończeniu pracy,
- procesy kończą działanie,
- zasoby IPC sprzątane po zakończeniu.
