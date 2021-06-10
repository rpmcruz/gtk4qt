# QMyStyle Makefile
# feel free to contribute with a nicer build system ;)

# EDIT: you must adjust the following var to your system
KDESTYLEDIR=/opt/kde3/lib/kde3/plugins/styles

CC = g++
MOC = $(QTDIR)/bin/moc
CFLAGS = -Wall `pkg-config --cflags qt-mt`
LIBS = `pkg-config --libs qt-mt`

OBJ = qsimplestyle.lo qmystyle.lo plugin.lo \
      moc_qsimplestyle.lo moc_qmystyle.lo

all: qmystyle.la
	@echo ""
	@echo "Done; now you must install the style by typing 'sudo make install'"
	@echo "You can then set it from kcontrol or qtconfig"

qsimplestyle.lo: qsimplestyle.cpp qsimplestyle.h
	libtool --mode=compile $(CC) $(CFLAGS) -c qsimplestyle.cpp

moc_qsimplestyle.lo: qsimplestyle.h
	$(MOC) qsimplestyle.h -o moc_qsimplestyle.cpp
	libtool --mode=compile $(CC) $(CFLAGS) -c moc_qsimplestyle.cpp

qmystyle.lo: qmystyle.cpp qmystyle.h qsimplestyle.h
	libtool --mode=compile $(CC) $(CFLAGS) -c qmystyle.cpp

moc_qmystyle.lo: qmystyle.h
	$(MOC) qmystyle.h -o moc_qmystyle.cpp
	libtool --mode=compile $(CC) $(CFLAGS) -c moc_qmystyle.cpp

plugin.lo: plugin.cpp qmystyle.h qsimplestyle.h
	libtool --mode=compile $(CC) $(CFLAGS) -c plugin.cpp

qmystyle.la: $(OBJ)
	libtool --mode=link $(CC) $(LIBS) -o qmystyle.la \
	        -rpath $(KDESTYLEDIR) -module $(OBJ)

install:
	libtool --mode=install /usr/bin/install -c -p qmystyle.la "$(KDESTYLEDIR)/qmystyle.la"

uninstall:
	libtool --mode=uninstall rm -f "$(KDESTYLEDIR)/qmystyle.la"

clean:
	rm -f moc_*.cpp qtapp *~ *.o *.lo *.la
	rm -rf .libs/
