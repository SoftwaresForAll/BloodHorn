#ifndef BLOODHORN_THEME_H
#define BLOODHORN_THEME_H
#include <stdint.h>

struct BootMenuTheme {
    uint32_t background_color;
    uint32_t header_color;
    uint32_t highlight_color;
    uint32_t text_color;
    uint32_t selected_text_color;
    uint32_t footer_color;
    void* background_image;
};

void SetBootMenuTheme(const struct BootMenuTheme* theme);
const struct BootMenuTheme* GetBootMenuTheme(void);

#endif // BLOODHORN_THEME_H 