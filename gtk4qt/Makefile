# Gtk4Qt's QGtkStyle Makefile
# feel free to contribute with a nicer build system ;)

# EDIT: you must adjust the following var to your system
KDESTYLEDIR=/opt/kde3/lib/kde3/plugins/styles

CC = g++
MOC = $(QTDIR)/bin/moc
CFLAGS = -Wall `pkg-config --cflags qt-mt gtk+-2.0`
LIBS = `pkg-config --libs qt-mt gtk+-2.0 gdk-pixbuf-xlib-2.0`

OBJ = qsimplestyle.lo qgtkstyle.lo plugin.lo \
      moc_qsimplestyle.lo moc_qgtkstyle.lo

all: qgtkstyle.la
	@echo ""
	@echo "Done; now you must install the style by typing 'sudo make install'"
	@echo "You can then set it from kcontrol or qtconfig"

qsimplestyle.lo: qsimplestyle.cpp qsimplestyle.h
	libtool --mode=compile $(CC) $(CFLAGS) -c qsimplestyle.cpp

moc_qsimplestyle.lo: qsimplestyle.h
	$(MOC) qsimplestyle.h -o moc_qsimplestyle.cpp
	libtool --mode=compile $(CC) $(CFLAGS) -c moc_qsimplestyle.cpp

qgtkstyle.lo: qgtkstyle.cpp qgtkstyle.h qsimplestyle.h
	libtool --mode=compile $(CC) $(CFLAGS) -c qgtkstyle.cpp

moc_qgtkstyle.lo: qgtkstyle.h
	$(MOC) qgtkstyle.h -o moc_qgtkstyle.cpp
	libtool --mode=compile $(CC) $(CFLAGS) -c moc_qgtkstyle.cpp

plugin.lo: plugin.cpp qgtkstyle.h qsimplestyle.h
	libtool --mode=compile $(CC) $(CFLAGS) -c plugin.cpp

qgtkstyle.la: $(OBJ)
	libtool --mode=link $(CC) $(LIBS) -o qgtkstyle.la \
	        -rpath $(KDESTYLEDIR) -module $(OBJ)

install:
	libtool --mode=install /usr/bin/install -c -p qgtkstyle.la "$(KDESTYLEDIR)/qgtkstyle.la"

uninstall:
	libtool --mode=uninstall rm -f "$(KDESTYLEDIR)/qgtkstyle.la"

clean:
	rm -f moc_*.cpp qtapp *~ *.o *.lo *.la
	rm -rf .libs/
