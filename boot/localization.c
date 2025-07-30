#include "localization.h"
#include "compat.h"
#include <string.h>

static const struct { const char* key; const wchar_t* en; const wchar_t* es; } table[] = {
    {"menu_title", L"BloodHorn Boot Menu", L"Menú de Arranque BloodHorn"},
    {"select", L"Select", L"Seleccionar"},
    {"boot", L"Boot", L"Arrancar"},
    {"exit", L"Exit", L"Salir"},
    {"instructions", L"↑/↓: Select  Enter: Boot  ESC: Exit", L"↑/↓: Seleccionar  Enter: Arrancar  ESC: Salir"},
    {NULL, NULL, NULL}
};

static const char* current_lang = "en";

void SetLanguage(const char* lang_code) {
    if (strcmp(lang_code, "es") == 0)
        current_lang = "es";
    else
        current_lang = "en";
}

const wchar_t* GetLocalizedString(const char* key) {
    for (int i = 0; table[i].key != NULL; ++i) {
        if (strcmp(table[i].key, key) == 0) {
            if (strcmp(current_lang, "es") == 0)
                return table[i].es;
            else
                return table[i].en;
        }
    }
    return L"";
}