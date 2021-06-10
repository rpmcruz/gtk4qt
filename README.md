# Gtk4Qt

**OLD CODE: This is an old project (2007) of mine (for GTK+2 and Qt3). Today Qt comes with QGtkStyle that does the same thing that my project did.**

## About

The goal of the project is to get KDE/Qt applications to render as if they were a GTK+ one, so to have them well integrated on Gnome, FLTK and other desktops where GTK+ is dominant.

KDE users should be able to benefit from this, but they may be better served with the [gtk-qt-engine](http://gtk-qt.ecs.soton.ac.uk/).

This software is not meant to be consumed just yet. The base is done and the effort now is to grow and polish the code. You don't need to learn or have much experience to help out programming.

![Screenshot 1](shot1.png)

![Screenshot 2](shot2.png)

![Screenshot 3](shot3.png)

## Running

Get the best and latest with: (sourceforge repo, now here)

You may test the style by typing "make" on the "test" sub-directory and then running "./qtapp".

To compile the code into a plugin, and so having it being applied to any KDE/Qt application, GDK (the drawing part of GTK+) must be patched. You may get the patch from [its bug report](https://bugzilla.gnome.org/show_bug.cgi?id=410010) (please, vote it up!)

It is patched against GTK+ 2.10.9, but might work on others.

Then, open the Makefile from the main directory and point the KDESTYLEDIR to the right location. Type "make", followed by a "make install" as root. (Yeah, I will look into using autoconf, cmake or something.) Have it applied using kcontrol's styles module: "kcmshell style"

## Hacking

The Qt style framework, QStyle, is pretty flexible, but is also a hasle to work with. Therefore, we have subclassed it into the QSimpleStyle framework that joins everything into two functions: paint() and dimension(). It is just an abstract class with the goal of shapping QStyle into something more similar to how GtkStyle works; it doesn't know of GTK+ at all however. [1] Then, we fill those virtual functions, with the derivated class, QGtkStyle.

[1] People that want to write Qt styles may want to consider using this framework, especially if they are considering to make a GTK+ style too.

For more information on QSimpleStyle, (see below).

It shouldn't be hard to understand, so just check the code! Before, check the Qt documentation on QStyle to get some insight on how a Qt style works.

There is plenty to hack on. Most of the work is to support more widgets and have current ones better coded (for instance, we probably don't want to leave a lot of stuff like subRect() for QCommonStyle). The bulk of the work has to do with Qt, not GTK+.
A sole GTK+ work to do is to have the style changing when the user changes it (currently the GTK+ style used is the one set when the class is constructed.)

If you want to colaborate, subscribe the mailing list and check the code. If you feel lost, don't hesitate to use the mailing list: (no longer available)

## QSimpleStyle

The need to specify the rectangular area of each subpart of a widget and having to draw widgets parts in such a non-modular way is cut down to two methods: one to draw simple primitives (like the arrow in a combo box), the other to specify certain attributes (like the border of said arrow).

Plus, this provides an uniform structure and "out of box" support for such things as high lighting. And you won't loose flexibility; in the worse case, you can always over-rule a QStyle function to do what you want and then pass it to QSimpleStyle for the rest.

Developers that want to port their styles from/to GTK+, may have a particular interest in this as the paradigm is similar to that of GTK+.

The reason why QStyle is complicated is because it has to be very well integrated into platforms such as Windows and MacOS, so it needs to be as generic as to allow it to be shapped into the platform's way of painting things (the target platform may have a paintSpinWidget() function and not have them distributed by functions such as paintSpinUpButton()).

For native styles, this complexity is unnecessary and results in not as good results, both visual and code wise.

Warning: this is still young work. But please use it, hack on it as necessary and send those changes back. :)

Get QSimpleStyle from: qsimplestyle.h and qsimplestyle.cpp

![Screenshot QSimpleStyle](qmystyle.png)

## FAQ

**Q: Why building this for Qt 3 and not Qt 4?**

**A:** To start with, the current KDE version still uses Qt 3, so that's what pretty much everyone will use on the Linux platform for at least a year, until KDE 4 gets deployed. And the drawing and style layers of Qt are not that different from where we stand [2], so porting shouldn't give much trouble.

[2] the fact that the drawing layer is more modular may take users to use the OpenGL backend, rather than the X11, which could create a problem for us. Hopefully, efforts like XGL will win out making X.org the one using OpenGL rather than libraries stop issuing their drawing commands through X11.

**Q: What's the performance hit by using this style?**

**A:** No performance test was done yet, but from the code I would say the cpu consuption is pretty much the same of any Qt style. There is only the cost of some more calls and simple structure conversions. GTK+ drawing and stuff should be of identical cost to that of Qt. The memory impact may be stronger, with some GTK+ widgets allocated in memory. Network usage, if the X client is at another computer that is, should be a bit heavier because we must do a server trip for each widget that does a draw request to create GdkPixmap for it. Complex clipping also requires more server trips, since we simplify it down into rectangles (GtkStyle only supports box clipping).

Overall, it really should be insignificant. Other factors like stability and good rendering ought be of more importance.

**Q: Are there any licensing issues? Like when running Opera with this style when it was compiled with GPL Qt?**

**A:** I am no lawyer, but no, I don't think there are no real licensing issues. This source code is under the LGPL, and so is GTK+. The LGPL doesn't make any restrictions in what you can link to it; except that if you statically link your program with this style, you must at least provide the object code of your program, so users may re-link it with a newer version of this style. Another restriction is that you must provide any modification you do to this style's source code under the LGPL.

For the Qt part; you should be safe as long as you use the open source Qt edition. That edition is licensed under both GPL and QPL licenses. While the GPL allows any GPL-compatible linking, QPL allows linking of software that uses the Qt proprietary edition. This means you should be clear to distribute both free and proprietary Qt programs with this style.
