/***************************************************************************
 *   Copyright (C) 2007 by Ricardo Cruz                                    *
 *   rpmcruz@clix.pt                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef QGTKSTYLE_H
#define QGTKSTYLE_H

#include "qsimplestyle.h"
#include <gtk/gtkwidget.h>

class QGtkStyle : public QSimpleStyle
{
Q_OBJECT

public:
	QGtkStyle();
	~QGtkStyle();

	virtual void paint (Paint what, WidgetKind kind, SFlags flags,
	                    QPainter *painter, const QColorGroup &colors,
	                    const QRect &rect, Side side = NoSide) const;
	// we need to handle QPainter clipping for ourselves, so we feed that to
	// this method, which can be called multiple times by paint():
	void paintClip (Paint what, WidgetKind kind, SFlags flags, const QRect &rect,
	                GdkRectangle *area, Side side) const;

	virtual int dimension (Dimension of, WidgetKind kind, Side side = NoSide) const;

	virtual void setTarget (Qt::HANDLE drawable) const;

	// from QStyle:
	virtual int styleHint (StyleHint stylehint, const QWidget *widget,
	                       const QStyleOption &opt, QStyleHintReturn *returnData) const;
	virtual QPixmap stylePixmap (StylePixmap stylepixmap, const QWidget *widget,
	                             const QStyleOption &opt) const;

	virtual void polish (QApplication *app);
	virtual void polish (QPalette &pal);

protected:
	// current drawable we are painting on (a GdkPixmap actually)
	GdkDrawable *drawable;
	// system colormap (loaded at first setTarget)
	// FIXME: we should try to set the current screen as default, and then
	// just get a reference to this through gdk_colormap_get_system()
	GdkColormap *colormap;

	// GtkStyle wants the widgets to paint to actually exist, so we cache them all
	// Some gtk themes seem to do fine when not fed with a proper widget on paint,
	// but we can't be sure that will work for every one.
	GtkWidget *getWidget (GType type) const;
	GtkWidget *getWidget (WidgetKind kind) const;

	// Qt <-> Gtk structures conversion
	static QColor toQtColor (const GdkColor &gcolor);
	static QPixmap toQtPixmap (GdkPixmap *gdk_pixmap, GdkGC *gdk_gc);
	static QPixmap toQtPixmap (GdkPixbuf *gdk_pixbuf, GdkGC *gdk_gc);

	static GdkRectangle toGdkRect (const QRect &rect);

	static void toGtkFlags (const SFlags how, GtkStateType *state,
	                        GtkShadowType *shadow);
	static void setWidgetState (GtkWidget *widget, const SFlags flags,
	                            GtkStateType state);

private:
	// Disabled copy constructor and operator=
	QGtkStyle (const QGtkStyle &);
	QGtkStyle& operator= (const QGtkStyle &);

	std::map <GType, GtkWidget *> widgetsCache;
	// dummies:
	GtkWidget *window_widget, *container_widget;
};

#endif /*QGTKSTYLE_H*/
