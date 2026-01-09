// logger.h
#pragma once

// Ustaw nazwe pliku logu (domyslnie "raport_symulacji.log")
void serwis_logger_set_file(const char* path);

// Zapis pojedynczej linii do logu (automatycznie dopisuje "\n")
void serwis_log(const char* prefix, const char* msg);

// Wersja do formatowania (printf-style)
void serwis_logf(const char* prefix, const char* fmt, ...);
