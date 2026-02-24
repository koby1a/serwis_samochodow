# Scenariusze testowe T1–T4

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

## T1 – 5000 samochodów tylko marki A (start synchroniczny)

Start:
```
./serwis_samochodow --config ../configs/T1.cfg
```

Weryfikacja:
- brak odrzuceń ofert (dla T1 akceptacja wymuszona),
- brak odrzuceń z tytułu godzin pracy,
- logi `A_ONLY fork` oraz nagły wzrost kolejek po starcie.

## T2 – 5000 krytycznych aut poza godzinami

Start:
```
./serwis_samochodow --config ../configs/T2.cfg
```

Weryfikacja:
- brak odrzuceń poza godzinami (wszystkie auta krytyczne),
- brak odrzuceń z powodu nieobsługiwanej marki (A,E,I,O,U,Y).

## T3 – Obciążenie raportów/kasy (IPC)

Start:
```
./serwis_samochodow --config ../configs/T3.cfg
```

Weryfikacja:
- intensywny strumień raportów/kasy,
- brak zacięć kolejek `rap` i `kasa`,
- poprawna kolejność logów płatności po zakończeniu (kasjer buforuje).

## T4 – Dodatkowe usterki (100% akceptacji)

Start:
```
./serwis_samochodow --config ../configs/T4.cfg
```

Weryfikacja:
- wszystkie dodatkowe usterki są akceptowane,
- brak zacięć w kanałach `ext_req` / `ext_resp`.
