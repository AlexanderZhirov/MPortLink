# MPortLink

**MPortLink** (*modem port link*) is designed to track connected USB modems and create permanent symbolic links to ports.

MPortLink was developed for direct use of several USB modems when connected to Asterisk telephony, the ports of which were randomly determined by the operating system. The utility allows you to create permanent symbolic links to devices, regardless of the order in which the ports of the USB modem device were defined.

Based on [ModemManager](https://gitlab.freedesktop.org/mobile-broadband/ModemManager).

## Dependency

The installed `modemmanager` is required for use.

## Build

`libmm-glib` library is required for the build. It is called differently in different distributions, for example: `libmm-glib-dev` or `libmm-glib-devel`.

After installing the dependencies, you just need to run the script `build.sh `and `mportlink` will be built in the `build` project directory.

### Manually

```
gcc -Werror -Wall -Os src/*.c `pkg-config --libs --cflags mm-glib` -o mportlink
```

## To-Do

Currently, the utility is linked to the ModemManager server, through which the connected modems are identified. It is planned to disconnect from this server, as there is a problem with the capture of the device and the unavailability of using Asterisk.
