# BloodHorn Bootloader Configuration

BloodHorn supports configuration via INI, JSON, and environment variables. Only real, working options are documented below.

## INI Configuration

Place a file named `bloodhorn.ini` in the boot partition root. Example:

```
[boot]
default=linux
menu_timeout=5

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

## Notes
- Only the above options are supported.
- Place config files in the root of the boot partition.
- If multiple config sources exist, INI > JSON > environment (INI has highest priority). 