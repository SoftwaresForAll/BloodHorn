# BloodHorn Bootloader Configuration

BloodHorn supports configuration via INI, JSON, and environment variables.

## INI Configuration

Place a file named `bloodhorn.ini` in the boot partition root. Example:

```
[boot]
default=linux
menu_timeout=5

[theme]
background_color=0x1A1A2E
header_color=0x2D2D4F
highlight_color=0x4A4A8A
text_color=0xCCCCCC
selected_text_color=0xFFFFFF
footer_color=0x8888AA
background_image=myimage.bmp

[localization]
language=en

[linux]
kernel=/boot/vmlinuz
initrd=/boot/initrd.img
cmdline=root=/dev/sda1 ro
```

## JSON Configuration

Place a file named `bloodhorn.json` in the boot partition root. Example:

```
{
  "boot": {
    "default": "linux",
    "menu_timeout": 5
  },
  "theme": {
    "background_color": "0x1A1A2E",
    "header_color": "0x2D2D4F",
    "highlight_color": "0x4A4A8A",
    "text_color": "0xCCCCCC",
    "selected_text_color": "0xFFFFFF",
    "footer_color": "0x8888AA",
    "background_image": "myimage.bmp"
  },
  "localization": {
    "language": "en"
  },
  "linux": {
    "kernel": "/boot/vmlinuz",
    "initrd": "/boot/initrd.img",
    "cmdline": "root=/dev/sda1 ro"
  }
}
```

## Environment Variables

Set environment variables before booting:

```
BLOODHORN_DEFAULT=linux
BLOODHORN_MENU_TIMEOUT=5
BLOODHORN_THEME_BACKGROUND_COLOR=0x1A1A2E
BLOODHORN_THEME_HEADER_COLOR=0x2D2D4F
BLOODHORN_THEME_HIGHLIGHT_COLOR=0x4A4A8A
BLOODHORN_THEME_TEXT_COLOR=0xCCCCCC
BLOODHORN_THEME_SELECTED_TEXT_COLOR=0xFFFFFF
BLOODHORN_THEME_FOOTER_COLOR=0x8888AA
BLOODHORN_THEME_BACKGROUND_IMAGE=myimage.bmp
BLOODHORN_LANGUAGE=en
BLOODHORN_LINUX_KERNEL=/boot/vmlinuz
BLOODHORN_LINUX_INITRD=/boot/initrd.img
BLOODHORN_LINUX_CMDLINE=root=/dev/sda1 ro
```

## Supported Options

- `default` — default boot entry (e.g. `linux`)
- `menu_timeout` — boot menu timeout in seconds
- `kernel` — path to kernel image
- `initrd` — path to initrd image
- `cmdline` — kernel command line
- `background_color`, `header_color`, `highlight_color`, `text_color`, `selected_text_color`, `footer_color`, `background_image` — theme options
- `language` — UI language (e.g. `en`, `es`)

## Notes
- Place config files in the root of the boot partition.
- If multiple config sources exist, INI > JSON > environment (INI has highest priority).
- For localization, you can add external language files (e.g. `lang_en.txt`).
- For custom themes, use a supported image format for `background_image`. 