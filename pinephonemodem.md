<title>Modem on PinePhone</title>
# Modem on PinePhone

Interacting with the modem on the PinePhone using a virtual serial port
via `picocom`.

## Using SXMO

These steps are for SXMO under postmarketOS which uses OpenRC over systemd,
if your phone is using systemd you may have to make changes... specifically
the service stopping and starting.

## Installing `picocom`

`picocom` is available in the [community edge](https://pkgs.alpinelinux.org/package/edge/community/aarch64/picocom)
Alpine repository and so can be installed through apk if the required
repository is available. Alternatively, compile from source, `picocom` does
not have many requirements. [https://github.com/npat-efault/picocom](https://github.com/npat-efault/picocom).

    apk add picocom

## Setup

These steps require a lot of typing and so it is suggested that the actions
below are performed via an ssh connection to the PinePhone.

To access the modem without strange things happening first disable the
ModemManager daemon:

    /etc/init.d/modemmanager stop
    /etc/init.d/eg25-manager stop
    /etc/init.d/gpsd stop

Stopping the above daemons will put the modem into power-down state (I think).

## Initialising the Modem

The example programme presented here will operate one pinephone v1.2... to
check that the programme is ok for your phone inspect the `/proc/device-tree/compatible`
file which, in my case looks like this:

    cat /proc/device-tree/compatible
    pine64,pinephone-1.2pine64,pinephoneallwinner,sun50i-a64

Notice 1.2. If the output is different modifications to the example programme
are required. See the `#defines` and `.toml` files in `/usr/share/eg25-manager`.

To initialise the modem I have written a simple programme that will open the
GPIO lines using the new interface (via libgpiod) and toggle the required
outputs to allow communication with the device via the hardware serial port
`/dev/ttyS2` and the USB serial port `/dev/ttyUSB2`.

These commands to manipulate the GPIO lines are more difficult since the
removal of the `/sys/class/gpio` interface but based on eg25-manager I have
written a programme (which will compile on the PinePhone) available
[here](pinephonemodem.c) (or a [binary](pinephonemodem)).

If you choose to compile you will need `gcc`, `musl-dev` and `libgpiod-dev`.

    apk add gcc musl-dev libgpiod-dev

Then compile as expected:

    gcc -o pinephonemodem pinephonemodem.c -lgpiod

To use this programme open picocom:

    picocom --baud 115200 --omap lfcr --flow h /dev/ttyS2

Then from another terminal window:

    ./pinephonemodem

Eventually there should be some output in the picocom window... try typing
`AT` and pressing enter.

Once `STATUS: 0` appears there should also be output in dmesg related to the
modem and USB device connection.

