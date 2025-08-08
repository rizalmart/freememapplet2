CC = gcc
CFLAGS = -Wall `pkg-config --cflags gtk+-3.0 ayatana-appindicator3-0.1`
LIBS = `pkg-config --libs gtk+-3.0 ayatana-appindicator3-0.1`
TARGET = freememapplet_tray
SRC = freememapplet_tray.c

# Translation
POT = freememapplet_tray.pot

# Install paths
PREFIX ?= /usr
BINDIR = $(PREFIX)/bin
PIXMAPDIR = $(PREFIX)/share/pixmaps/freememapplet

SVG_FILES = container_0.svg container_1.svg container_2.svg container_3.svg container_4.svg

all: $(TARGET) $(POT)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	strip --strip-unneeded $@

$(POT): $(SRC)
	xgettext --keyword="_" $(SRC) -o $(POT)

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/

	install -d $(DESTDIR)$(PIXMAPDIR)
	install -m 644 $(SVG_FILES) $(DESTDIR)$(PIXMAPDIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	for f in $(SVG_FILES); do rm -f $(DESTDIR)$(PIXMAPDIR)/$$f; done
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(PIXMAPDIR)

clean:
	rm -f $(TARGET) $(POT)

.PHONY: all install uninstall clean
