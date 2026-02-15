# INFO

Main purpose of this tool is to provide easy interface to manage keyboard backlight.

# My setup

I use Logitech G5, which has tuxedo-drivers compatible keyboard.
This drivers expose rgb:kbd_backlight device in /sys/class/leds

# Usage

```sh
'Where level is unsigned 8bit int (0..255) changes brightness level of keyboard,
<12 seems to have strange flashing effect'
kbd-rgb LEVEL
'Where each is unsigned 8bit int (0..255) changes brightness level of each color.
0 and 255 are stable, something in between can be unstable,
you need to test it for yourself'
kbd-rgb R G B
'Returns brightness levels of all colors in "R G B" format,
where each is unsigned 8bit int'
kbd-rgb --get
```
