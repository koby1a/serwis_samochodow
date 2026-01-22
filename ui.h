#pragma once

/** @brief ANSI kolor: czerwony. */
const char* ui_red();
/** @brief ANSI kolor: zielony. */
const char* ui_green();
/** @brief ANSI kolor: zolty. */
const char* ui_yellow();
/** @brief ANSI kolor: niebieski. */
const char* ui_blue();
/** @brief ANSI kolor: magenta. */
const char* ui_magenta();
/** @brief ANSI kolor: cyan. */
const char* ui_cyan();
/** @brief ANSI kolor: szary. */
const char* ui_gray();
/** @brief ANSI reset stylu. */
const char* ui_reset();

/** @brief Czysci ekran terminala. */
void ui_clear();
/** @brief Ustawia kursor na poczatek (home). */
void ui_home();
