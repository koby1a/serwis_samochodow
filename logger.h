#pragma once

/**
 * @brief Ustawia sciezke do pliku logu.
 * @param path Sciezka do pliku. Gdy nullptr lub pusty, ignoruje.
 */
void serwis_logger_set_file(const char* path);

/**
 * @brief Usuwa plik logu (unlink), aby zaczac od zera.
 */
void serwis_logger_reset_file();

/**
 * @brief Zapisuje pojedyncza linie do logu: [prefix] msg
 * @param prefix Prefiks (np. nazwa procesu).
 * @param msg Tresc.
 */
void serwis_log(const char* prefix, const char* msg);

/**
 * @brief Zapisuje formatowana linie do logu (printf-style).
 * @param prefix Prefiks.
 * @param fmt Format.
 */
void serwis_logf(const char* prefix, const char* fmt, ...);
