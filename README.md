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

## Using

Superuser rights are required to work. Start the daemon and connect your USB modem. In `journalctl` you will see his work. Links to the ports of the connected modem will be created in the `/dev/dongle` directory.

```
Dec 21 13:51:02 solus mportlink[17764]: main: starting the mportlink daemon
Dec 21 13:51:26 solus mportlink[17764]: mpl_symlink_ports, dst_path: the symbolic link has been successfully created [/dev/dongle/358**********26-0 -> /dev/ttyUSB0]
Dec 21 13:51:26 solus mportlink[17764]: mpl_symlink_ports, dst_path: the symbolic link has been successfully created [/dev/dongle/358**********26-1 -> /dev/ttyUSB1]
Dec 21 13:51:41 solus mportlink[17764]: mpl_unlink_ports, dst_path: link was successfully deleted [/dev/dongle/358**********26-0]
Dec 21 13:51:41 solus mportlink[17764]: mpl_unlink_ports, dst_path: link was successfully deleted [/dev/dongle/358**********26-1]
Dec 21 13:51:48 solus mportlink[17764]: signals_handler: cancelling the operation...
Dec 21 13:51:48 solus mportlink[17764]: signals_handler: cancelling the main loop...
Dec 21 13:51:48 solus mportlink[17764]: main: stopping the mportlink daemon
```

## To-Do

Currently, the utility is linked to the ModemManager server, through which the connected modems are identified. It is planned to disconnect from this server, as there is a problem with the capture of the device and the unavailability of using Asterisk.
