#include "theme.h"
#include "compat.h"

static struct BootMenuTheme current_theme = {
    .background_color = 0x1A1A2E,
    .header_color = 0x2D2D4F,
    .highlight_color = 0x4A4A8A,
    .text_color = 0xCCCCCC,
    .selected_text_color = 0xFFFFFF,
    .footer_color = 0x8888AA,
    .background_image = NULL
};

void SetBootMenuTheme(const struct BootMenuTheme* theme) {
    memcpy(&current_theme, theme, sizeof(struct BootMenuTheme));
}

const struct BootMenuTheme* GetBootMenuTheme(void) {
    return &current_theme;
} 