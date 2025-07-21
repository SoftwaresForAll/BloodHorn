#include "localization.h"
#include <string.h>

static const struct { const char* key; const wchar_t* en; const wchar_t* es; } table[] = {
    {"menu_title", L"BloodHorn Boot Menu", L"Menú de Arranque BloodHorn"},
    {"select", L"Select", L"Seleccionar"},
    {"boot", L"Boot", L"Arrancar"},
    {"exit", L"Exit", L"Salir"},
    {"instructions", L"↑/↓: Select  Enter: Boot  ESC: Exit", L"↑/↓: Seleccionar  Enter: Arrancar  ESC: Salir"},
    {NULL, NULL, NULL}
};

static const wchar_t* current_lang = L"en";

void SetLanguage(const char* lang_code) {
    if (strcmp(lang_code, "es") == 0) current_lang = L"es";
    else current_lang = L"en";
}

const wchar_t* GetLocalizedString(const char* key) {
    for (int i = 0; table[i].key; ++i) {
        if (strcmp(table[i].key, key) == 0) {
            if (current_lang == L"es") return table[i].es;
            return table[i].en;
        }
    }
    return L"";
} 