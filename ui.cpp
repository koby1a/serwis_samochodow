#include "ui.h"
#include <iostream>

const char* ui_red()     { return "\033[31m"; }
const char* ui_green()   { return "\033[32m"; }
const char* ui_yellow()  { return "\033[33m"; }
const char* ui_blue()    { return "\033[34m"; }
const char* ui_magenta() { return "\033[35m"; }
const char* ui_cyan()    { return "\033[36m"; }
const char* ui_gray()    { return "\033[90m"; }
const char* ui_reset()   { return "\033[0m";  }

void ui_clear() { std::cout << "\033[2J"; }
void ui_home()  { std::cout << "\033[H";  }
