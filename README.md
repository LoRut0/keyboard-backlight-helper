# INFO

Main purpose of this tool is to provide easy interface to manage keyboard backlight.

# My setup

I use Logitech G5, which has tuxedo-drivers compatible keyboard.
This drivers expose rgb:kbd_backlight device in /sys/class/leds

# Usage

```sh
kbd-rgb LEVEL
kbd-rgb +LEVEL / -LEVEL
'Where level is unsigned 8bit int (0..255) changes brightness level of keyboard,
<12 seems to have strange flashing effect'
kbd-rgb R G B
'Where each is unsigned 8bit int (0..255) changes brightness level of each color.
0 and 255 are stable, something in between can be unstable,
you need to test it for yourself'
kbd-rgb --level
'Returns brightness level, which is unsigned 8bit int'
kbd-rgb --color
'Returns brightness levels of all colors in "R G B" format,
where each is unsigned 8bit int'
```

# Install

```sh
# By default user must be in "kbdlight" group, so you should add yourself to this group
# You can change this group by changing ALLOW_GROUP inside Makefile
sudo groupadd -f kbdlight
sudo usermod -aG kbdlight $USER
# Check SYSFS_PATH and ALLOW_GROUP inside
make install # Builds and Installs kbd-rgb binary (default to /usr/local/bin

# To uninstall
make uninstall
```
