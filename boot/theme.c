/*
 * BloodHorn Bootloader
 *
 * This file is part of BloodHorn and is licensed under the MIT License.
 * See the root of the repository for license details.
 */
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
// ehhhh string.h and etc don't work in these stages of booting. no libc. no native header. but don't worry. my compat.h will handle this
void SetBootMenuTheme(const struct BootMenuTheme* theme) {
    memcpy(&current_theme, theme, sizeof(struct BootMenuTheme));
}
const struct BootMenuTheme* GetBootMenuTheme(void) {
    return &current_theme;
} 