# hawk
[![Build Status](https://travis-ci.com/patrickp89/hawk.svg?branch=master)](https://travis-ci.com/patrickp89/hawk)

A linux device driver for toy USB missile launchers.

## How to build it
### Driver
Install *gcc*, *make*, and the header files for your kernel. On Debian run:
```bash
# apt-get install build-essential linux-headers-$(uname -r)
```

Clone this repository, change into the "driver" folder and compile the driver:
```bash
$ cd driver/
$ make
```

Insert the compiled module into the kernel:
```bash
# insmod hawk.ko
```

You can check whether the module was loaded correctly by either searching the syslog
```bash
# grep "hawk.*module.*loaded" /var/log/syslog
```
or running
```bash
# lsmod | grep hawk
```

### User interface
Install *ncurses* and its header files. On Debian run:
```bash
# apt-get install libncurses5 libncurses5-dev
```

Change into the user interface folder and run make:
```bash
$ cd ../gui/
$ make
```

## How to use it
Once you have compiled both the driver itself and the user interface, loaded the driver module into the kernel, and verified that the module was loaded correctly, you are ready to go: Grab your toy missile launcher and plug it into a USB port.

You can verify that the hawk driver did detect your missile launcher by checking the syslog:
```bash
# grep "hawk.*device.*attached" /var/log/syslog
```
If your device was not detected, you can try and alter the device table in the driver's [source file](driver/hawk.c).

Change into the user interface folder and start the user interface:
```bash
$ cd ../gui/
$ ./missile_control
```

---
>***Warning! The toy missile launchers that can be operated with this driver are mechanically constraint in their movements: the turrets do, for instance, not rotate by full 360° (let alone infinitely). The current driver version DOES NOT cancel movements (e.g. a clockwise rotation) by itself! If you trigger an action / a movement, make sure to stop it before any mechanical harm is inflicted!***
>***This open-source driver is an educational project: the author does not offer any warranties or liabilities for usage of this software!***
---

You can now operate your missile launcher by simply pressing one of the folowing buttons on your keyboard:
```
[a] rotate left
[d] rotate right
[w] aim up
[s] aim down
[f] fire missiles
[q] stop movement
[x] quit program
```
Make sure to cancel all actions/movements by pressing [q]! Quit the GUI by pressing [x].
