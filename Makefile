# ===== Config =====
PREFIX      ?= /usr/local
BINDIR      ?= $(PREFIX)/bin
TARGET      := kbd-rgb
SRC         := kbd.c

# Override at build time if needed:
# make SYSFS_PATH=/sys/class/leds/rgb::kbd_backlight
SYSFS_PATH  ?= /sys/class/leds/rgb:kbd_backlight

# Group allowed to execute helper
ALLOW_GROUP ?= kbdlight

# ===== Build flags =====
CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -D SYSFS_PATH=\"$(SYSFS_PATH)\"

# ===== Targets =====

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

install: $(TARGET)
	sudo install -o root -g $(ALLOW_GROUP) -m 4750 $(TARGET) $(BINDIR)/$(TARGET)
	sudo chmod u+s $(BINDIR)/$(TARGET)
	@echo "Installed to $(BINDIR)/$(TARGET)"
	@echo "Make sure your user is in group: $(ALLOW_GROUP)"

uninstall:
	sudo rm -f $(BINDIR)/$(TARGET)
	@echo "Removed $(BINDIR)/$(TARGET)"

clean:
	rm -f $(TARGET)

reinstall: uninstall install

.PHONY: all install uninstall clean reinstall
