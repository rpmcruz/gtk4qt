/***************************************************************************
 *   This code is public domain provided as an example of usage of the     *
 *   QSimpleStyle library.                                                 *
 *   by Ricardo Cruz                                                       *
 ***************************************************************************/

#ifndef QMYSTYLE_H
#define QMYSTYLE_H

#include "qsimplestyle.h"

class QMyStyle : public QSimpleStyle
{
Q_OBJECT

public:
	QMyStyle();
	~QMyStyle();

	virtual void paint (Paint what, WidgetKind kind, SFlags flags,
	                    QPainter *painter, const QColorGroup &colors,
	                    const QRect &rect, Side side = NoSide) const;

	virtual int dimension (Dimension of, WidgetKind kind, Side side = NoSide) const;

protected:
	void paintShadowRect (QPainter *painter, const QRect &rect,
	                      const QColor &light, const QColor &dark,
	                      bool invert = false, const QColor *fill = 0,
	                      bool paint_bottom = true) const;

	QColor shade (const QColor &color, float rate) const;

private:
	// Disabled copy constructor and operator=
	QMyStyle (const QMyStyle &);
	QMyStyle& operator= (const QMyStyle &);
};

#endif /*QMYSTYLE_H*/
