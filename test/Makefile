# Gtk4Qt static application test Makefile

CC = g++
MOC = $(QTDIR)/bin/moc
CFLAGS = -Wall `pkg-config --cflags qt-mt gtk+-2.0`
LIBS = `pkg-config --libs qt-mt gtk+-2.0 gdk-pixbuf-xlib-2.0`

OBJ = qtapp.o qsimplestyle.o qgtkstyle.o \
      moc_qtapp.o moc_qsimplestyle.o moc_qgtkstyle.o

all: qtapp
	@echo "Done"
	@echo "  -> Run ./qtapp to try this out! <-"

qtapp.o: qtapp.cpp
	$(CC) $(CFLAGS) -c qtapp.cpp -o qtapp.o

moc_qtapp.o: qtapp.h
	$(MOC) qtapp.h -o moc_qtapp.cpp
	$(CC) $(CFLAGS) -c moc_qtapp.cpp -o moc_qtapp.o

qsimplestyle.o: ../qsimplestyle.cpp ../qsimplestyle.h
	$(CC) $(CFLAGS) -c ../qsimplestyle.cpp -o qsimplestyle.o

moc_qsimplestyle.o: ../qsimplestyle.h
	$(MOC) ../qsimplestyle.h -o moc_qsimplestyle.cpp
	$(CC) $(CFLAGS) -c moc_qsimplestyle.cpp -o moc_qsimplestyle.o

qgtkstyle.o: ../qgtkstyle.cpp ../qgtkstyle.h ../qsimplestyle.h
	$(CC) $(CFLAGS) -c ../qgtkstyle.cpp -o qgtkstyle.o

moc_qgtkstyle.o: ../qgtkstyle.h
	$(MOC) ../qgtkstyle.h -o moc_qgtkstyle.cpp
	$(CC) $(CFLAGS) -c moc_qgtkstyle.cpp -o moc_qgtkstyle.o

qtapp: $(OBJ)
	$(CC) $(LIBS) $(OBJ) -o qtapp

clean:
	rm -f $(OBJ) moc_*.cpp qtapp *~
