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

/***************************************************************************
 * This is an abstract class that wraps QStyle to shape it into a simpler  *
 * form, based on GTK+'s. We also support highlighting natively, while you *
 * need to implement that yourself when using QStyle.                      *
 *                                                                         *
 * Really, QStyle API is certainly needed so that Qt apps can be so much   *
 * better integrated into different systems looks as opposite to other     *
 * libraries, but is just a hassle to code a style for and results in poor *
 * code. This layer could be good for style writters, especially, cross    *
 * desktop ones.                                                           *
 ***************************************************************************/

#ifndef QSIMPLESTYLE_H
#define QSIMPLESTYLE_H

#include <qcommonstyle.h>

class QSimpleStyle : public QCommonStyle
{
Q_OBJECT

public:
	QSimpleStyle();
	~QSimpleStyle();

	//** Our methods -- overload the virtual stuff

	enum Side { NoSide = 0, LeftSide, RightSide, TopSide, BottomSide };
	enum WidgetKind {
		UNSPECIFIED_WIDGET = 0, BUTTON_WIDGET, RADIO_WIDGET, CHECK_WIDGET,
		COMBO_WIDGET, COMBO_ENTRY_WIDGET, LISTVIEW_WIDGET, LINE_EDIT_WIDGET,
		TEXT_VIEW_WIDGET, SCROLL_HBAR_WIDGET, SCROLL_VBAR_WIDGET, HSLIDER_WIDGET,
		VSLIDER_WIDGET, SPIN_WIDGET, MENU_BAR_WIDGET, MENU_WINDOW_WIDGET,
		MENU_ITEM_WIDGET, TOOL_ITEM_WIDGET, NOTEBOOK_WIDGET, PROGRESS_BAR_WIDGET,
	};

	// Drawing -- use the passed QPainter and QColorGroup to paint the widget
	// part requested by our Paint enum in the rect passed. The widget type
	// is passed so you may personalize the painting. The SFlags specifies
	// the widget's state (eg: active, disabled) and other information like
	// its orientation (ie. flags & Style_Horizontal); for more details check
	// the QStyle documentation. Our Side enum specifies the widget's part side
	// when it makes sense; eg. the step button of a vertical scroll bar can be
	// either on TopSide or BottomSide.

	enum Paint {
		BUTTON_PAINT, CHECK_INDICATOR_PAINT, RADIO_INDICATOR_PAINT, FOCUS_PAINT,
		FRAME_PAINT, BACKGROUND_PAINT, HIGHLIGHT_PAINT, ENTRY_PAINT, ARROW_PAINT,
		SPIN_BUTTONS_BOX_PAINT, SCROLL_HANDLE_PAINT, SCROLL_GROOVE_PAINT,
		SIZE_GRIP_PAINT, SEPARATOR_PAINT, SPLITTER_PAINT, HEADER_PAINT,
		PROGRESS_BAR_PAINT
	};

	virtual void paint (Paint what, WidgetKind kind, SFlags flags,
	                    QPainter *painter, const QColorGroup &colors,
	                    const QRect &rect, Side side = NoSide) const {}

	// Layout -- specifies the different lengths to be allocated for several
	// widgets parts. Check the paint() comment on WidgetKind and Side.

	enum Dimension {
		WIDGET_THICKNESS_DIM, FOCUS_BORDER_DIM, FOCUS_WIDTH_DIM, CHILD_BORDER_DIM,
		FOCUS_DOWN_SHIFT_DIM, CHILD_DOWN_SHIFT_DIM, BUTTON_BORDER_DIM,
		BUTTON_DEFAULT_BORDER_DIM, INDICATOR_BORDER_DIM, INDICATOR_WIDTH_DIM,
		INDICATOR_HEIGHT_DIM, COMBO_ARROW_BORDER_DIM, COMBO_ARROW_WIDTH_DIM,
		SPIN_ARROW_WIDTH_DIM, SLIDER_FRAME_WIDTH_DIM, SLIDER_HANDLE_WIDTH_DIM,
		SLIDER_HANDLE_MIN_LENGTH_DIM, SLIDER_TICKS_SPACE_DIM, SCROLL_STEP_BUTTON_BORDER_DIM,
		SCROLL_STEP_BUTTON_WIDTH_DIM, SCROLL_SLIDER_BORDER_DIM, SPLITTER_WIDTH_DIM,
		MENU_ITEM_SPACING_DIM, MENU_ITEM_ARROW_SPACING_DIM, MENU_ITEM_ICON_SPACING_DIM,
		MENU_ITEM_SEPARATOR_HEIGHT_DIM, MENU_ITEM_CHECK_SIZE_DIM,
		POPUP_MENU_VERTICAL_BORDER_DIM, NOTEBOOK_TAB_OVERLAP_DIM,
	};

	virtual int dimension (Dimension of, WidgetKind kind, Side side = NoSide) const
	{ return 0; }

	// You don't want to use this. It tells the X11 the Drawable to paint on; 
	// only useful for system wrapper styles. They could just got it from QPainter
	// on every paint, but this is more efficient as more than one painting may
	// be needed to draw a widget (on X11 telling the wrapping library about this
	// will likely result in server trips).
	virtual void setTarget (Qt::HANDLE drawable) const {}

	// you may also want to overload styleHint() and stylePixmap() from QStyle

	//** QStyle methods (you shouldn't need to overload these -- if you do, call
	// QSimpleStyle::method() from your overload):
	virtual void polish (QWidget *widget);
	virtual void unPolish (QWidget *widget);
	virtual void polishPopupMenu (QPopupMenu *popup) {}

	virtual void drawPrimitive (PrimitiveElement pe, QPainter *p, const QRect &r,
	                            const QColorGroup &cg, SFlags flags = Style_Default,
	                            const QStyleOption &opt = QStyleOption::Default) const;
	virtual void drawControl (ControlElement element, QPainter *p, const QWidget *widget,
	                          const QRect &r, const QColorGroup &cg, SFlags how,
	                          const QStyleOption &opt) const;
	virtual void drawControlMask (ControlElement element, QPainter *p,
	                              const QWidget *widget, const QRect &r,
	                              const QStyleOption &opt) const;
	virtual void drawComplexControl (ComplexControl control, QPainter *p,
	                                 const QWidget *widget, const QRect &r,
	                                 const QColorGroup &cg, SFlags how,
	                                 SCFlags sub, SCFlags subActive,
	                                 const QStyleOption &opt) const;
	virtual void drawComplexControlMask (ComplexControl control, QPainter *p,
	                                     const QWidget *widget, const QRect &r,
	                                     const QStyleOption &opt) const;

	int pixelMetric (PixelMetric metric, const QWidget *widget) const;
	QRect subRect (SubRect subrect, const QWidget *widget) const;
	virtual QRect querySubControlMetrics (ComplexControl control, const QWidget *widget,
		SubControl subcontrol, const QStyleOption &opt = QStyleOption::Default) const;
	virtual QSize sizeFromContents (ContentsType contents, const QWidget *widget,
	                                const QSize &contentsSize, const QStyleOption &opt) const;

	virtual SubControl querySubControl (ComplexControl control, const QWidget *widget,
		const QPoint &pos, const QStyleOption &opt = QStyleOption::Default) const;

	// utilities
	// TODO: swap the last two arguments
	void apply_style_border (QRect &rect, WidgetKind kind, Dimension dim) const;
	void apply_style_size (QSize &size, WidgetKind kind, Dimension dim) const;
	void apply_style_shift (QRect &rect, WidgetKind kind, Dimension dim) const;

protected:
	// Highlight support
	SubControl hover_subcontrol;

private:
	// Disabled copy constructor and operator=
	QSimpleStyle (const QSimpleStyle &);
	QSimpleStyle& operator= (const QSimpleStyle &);

	bool eventFilter (QObject *object, QEvent *event);
};

// utility macros
inline void add_hor_border (QRect &rect, int b)
{ rect.addCoords (b, 0, -b, 0); }
inline void add_ver_border (QRect &rect, int b)
{ rect.addCoords (0, b, 0, -b); }
inline void add_border (QRect &rect, int b)
{ rect.addCoords (b, b, -b, -b); }

#endif /*QSIMPLESTYLE_H*/
