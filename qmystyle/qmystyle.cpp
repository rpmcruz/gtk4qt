/***************************************************************************
 *   This code is public domain provided as an example of usage of the     *
 *   QSimpleStyle library.                                                 *
 *   by Ricardo Cruz                                                       *
 ***************************************************************************/

#include <qpalette.h>
#include <qpainter.h>

#include "qmystyle.h"

#define HIGHLIGHT_SHADE 1.040
// for shadow
#define LIGHT_SHADE     1.320
#define DARK_SHADE      0.700

QMyStyle::QMyStyle() : QSimpleStyle()
{
}

QMyStyle::~QMyStyle()
{
}

void QMyStyle::paint (Paint paint_what, WidgetKind kind, SFlags flags, QPainter *painter,
                      const QColorGroup &colors, const QRect &rect, Side side) const
{
	painter->save();
	painter->setPen (NoPen);
	painter->setBrush (NoBrush);

	int x, y, w, h;
	rect.rect (&x, &y, &w, &h);

	switch (paint_what) {
		case BUTTON_PAINT:
		{
			QColor base (colors.button());
			if (flags & (Style_Down|Style_Selected))
				base = colors.light();
			else if (!(flags & Style_Enabled))
				base = colors.background();
			else if (flags & Style_MouseOver)
				base = shade (base, HIGHLIGHT_SHADE);

			if (kind == BUTTON_WIDGET) {
				// draw an extra border
				if (flags & Style_ButtonDefault)
					paintShadowRect (painter, rect, colors.shadow(), colors.shadow());
				else
					paintShadowRect (painter, rect, colors.mid(), colors.light());
				paintShadowRect (painter, QRect (x+1, y+1, w-2, h-2),
				                 colors.dark(), colors.dark());
				paintShadowRect (painter, QRect (x+2, y+2, w-4, h-4),
				                 colors.light(), colors.mid(),
				                 flags & Style_Down, &base);
			}
			else
				paintShadowRect (painter, rect, colors.light(), colors.mid(),
				                 flags & Style_Down, &base);
			break;
		}
		case CHECK_INDICATOR_PAINT:
		{
			// make them always the same size
			w = 3+10;
			h = 3+10;
			x += (rect.width() - w) / 2;
			y += (rect.height() - h) / 2;

			QColor base (colors.base());
			if (flags & Style_Down)
				base = colors.mid();

			paintShadowRect (painter, QRect (x, y, w, h), colors.mid(), colors.light());
			paintShadowRect (painter, QRect (x+1, y+1, w-2, h-2),
			                 colors.dark(), colors.dark(), false, &base);

			if (flags & Style_On) {
				painter->setPen (QPen (colors.shadow(), 2, SolidLine, SquareCap, MiterJoin));
				painter->drawLine (x+4, y+4, x+w-5, y+w-5);
				painter->drawLine (x+w-5, y+4, x+4, y+w-5);
			}
			break;
		}
		case RADIO_INDICATOR_PAINT:
		{
			// make them always the same size
			w = 3+10;
			h = 3+10;
			x += (rect.width() - w) / 2;
			y += (rect.height() - h) / 2;

			QColor base (colors.base());
			if (flags & Style_Down)
				base = colors.mid();

			painter->setPen (QPen (colors.mid(), 1, SolidLine, SquareCap, MiterJoin));
			painter->drawArc (x, y, w, h, 45*16, 180*16);
			painter->setPen (QPen (colors.light(), 1, SolidLine, SquareCap, MiterJoin));
			painter->drawArc (x, y, w, h, 225*16, 180*16);

			painter->setPen (QPen (colors.dark(), 1, SolidLine, SquareCap, MiterJoin));
			painter->setBrush (QBrush (base));
			painter->drawEllipse (x+1, y+1, w-2, h-2);

			if (flags & Style_On) {
				painter->setPen (NoPen);
				painter->setBrush (QBrush (QColor (colors.shadow())));
			painter->drawEllipse (x+4, y+4, w-8, h-8);
			}
			break;
		}
		case FOCUS_PAINT:
			painter->setPen (QPen (colors.dark(), 1, DotLine));
			painter->drawRect (rect);
			break;

		case SEPARATOR_PAINT:
			if (flags & Style_Horizontal) {
				painter->setPen (QPen (colors.dark(), 1));
				painter->drawLine (x, y, x+w, y);

				painter->setPen (QPen (colors.light(), 1));
				painter->drawLine (x, y+1, x+w, y+1);
			}
			else {
				painter->setPen (QPen (colors.dark(), 1));
				painter->drawLine (x, y, x, y+h);

				painter->setPen (QPen (colors.light(), 1));
				painter->drawLine (x+1, y, x+1, y+h);
			}
			break;
		case FRAME_PAINT:
		case BACKGROUND_PAINT:
			if (kind == UNSPECIFIED_WIDGET) {
				painter->setPen (QPen (colors.dark()));
				painter->drawRect (QRect (x, y, w-1, h-1));

				// light shadow
				painter->setPen (QPen (colors.light()));
				painter->drawLine (x+1, y+1, x+w-3, y+1);  // top
				painter->drawLine (x+w-1, y, x+w-1, y+h-2);  // right
				painter->drawLine (x, y+h-1, x+w-1, y+h-1);  // bottom
				painter->drawLine (x+1, y+1, x+1, y+h-3);  // left
			}
			else if (kind == NOTEBOOK_WIDGET) {
				paintShadowRect (painter, rect, colors.light(), colors.dark());
			}
			else {
				paintShadowRect (painter, rect, colors.dark(), colors.dark());
				paintShadowRect (painter, QRect (x+1, y+1, w-2, h-2), colors.light(), colors.mid());
			}
			break;

		case HIGHLIGHT_PAINT:
			if (kind == MENU_ITEM_WIDGET)
				painter->setBrush (QBrush (colors.highlight()));
			else
				painter->setBrush (QBrush (shade (colors.background(), HIGHLIGHT_SHADE)));
			painter->drawRect (rect);
			break;

		case ENTRY_PAINT:
			paintShadowRect (painter, rect, colors.dark(), colors.dark());
			paintShadowRect (painter, QRect (x+1, y+1, w-2, h-2), colors.light(), colors.mid(),
			                 false, &colors.base());
			break;

		case ARROW_PAINT:
		{
			// w must be even
			w = w - w%2;
			// wxh ratio must be 2x1
			if (w > h*2)
				w = h*2;
			else
				h = (int)((w+0.5)/2);
			x += (rect.width() - w) / 2;
			y += (rect.height() - h) / 2;

			painter->setPen (QPen (colors.shadow()));
			painter->setBrush (QBrush (colors.shadow()));
			QPointArray points (3);
			switch (side) {
				case TopSide:
					points.setPoint (0, x + w/2, y);
					points.setPoint (1, x, y + h);
					points.setPoint (2, x + w, y + h);
					break;
				case BottomSide:
				default:  // should never happen
					points.setPoint (0, x + w/2, y + h);
					points.setPoint (1, x, y);
					points.setPoint (2, x + w, y);
					break;
				case LeftSide:
					points.setPoint (0, x, y + h/2);
					points.setPoint (1, x + w, y);
					points.setPoint (2, x + w, y + h);
					break;
				case RightSide:
					points.setPoint (0, x + w, y + h/2);
					points.setPoint (1, x, y);
					points.setPoint (2, x, y + h);
					break;
			}
			painter->drawPolygon (points);
			break;
		}
		case SPIN_BUTTONS_BOX_PAINT:
			break;
		case SCROLL_HANDLE_PAINT:
		{
			QColor base (colors.highlight());
			paintShadowRect (painter, rect, shade (base, LIGHT_SHADE),
			                 shade (base, DARK_SHADE), false, &base);
			break;
		}
		case SCROLL_GROOVE_PAINT:
			if (kind == HSLIDER_WIDGET || kind == VSLIDER_WIDGET)
				paint (ENTRY_PAINT, kind, flags, painter, colors, rect, side);
			else
				paint (BUTTON_PAINT, kind, flags, painter, colors, rect, side);
			break;
		case SIZE_GRIP_PAINT:
			break;
		case SPLITTER_PAINT:
			break;
		case HEADER_PAINT:
			paintShadowRect (painter, rect, colors.light(), colors.dark(),
			                 false, NULL, false);
			if (flags & Style_Selected)
				painter->fillRect (x+w-3, y+1, 2, h-1, QBrush (colors.highlight()));
			break;
		case PROGRESS_BAR_PAINT:
			paint (SCROLL_HANDLE_PAINT, kind, flags, painter, colors, rect, side);
			break;
	}
	painter->restore();
}

void QMyStyle::paintShadowRect (QPainter *painter, const QRect &rect,
                                const QColor &light_clr, const QColor &dark_clr,
                                bool invert, const QColor *fill_clr,
                                bool paint_bottom) const
{
	int x, y, w, h;
	rect.rect (&x, &y, &w, &h);

	QBrush light (invert ? dark_clr : light_clr),
	       dark  (invert ? light_clr : dark_clr);
	painter->fillRect (x, y, w, 1, light);
	painter->fillRect (x, y, 1, h, light);
	painter->fillRect (x + w - 1, y+1, 1, h-1, dark);
	if (paint_bottom)
		painter->fillRect (x+1, y+h-1, w-1, 1, dark);

	if (fill_clr)
		painter->fillRect (x+1, y+1, w-2, h-2, QBrush (*fill_clr));
}

QColor QMyStyle::shade (const QColor &c, float n) const
{
	return QColor ((int)(c.red() * n), (int)(c.green() * n), (int)(c.blue() * n));
}

int QMyStyle::dimension (Dimension dim, WidgetKind kind, Side side) const
{
	switch (dim) {
		case WIDGET_THICKNESS_DIM:
			if (kind == BUTTON_WIDGET)
				return 4;
			if (kind == PROGRESS_BAR_WIDGET)
				return 2;
			return 0;
		case FOCUS_BORDER_DIM:
		case CHILD_BORDER_DIM:
			return 2;
		case FOCUS_DOWN_SHIFT_DIM:
		case CHILD_DOWN_SHIFT_DIM:
			return 1;
		case BUTTON_BORDER_DIM:
		case BUTTON_DEFAULT_BORDER_DIM:
			return 0;

		case INDICATOR_BORDER_DIM:
			return 4;
		case INDICATOR_WIDTH_DIM:
		case INDICATOR_HEIGHT_DIM:
			return 13;

		case COMBO_ARROW_BORDER_DIM:
			return 4;
		case COMBO_ARROW_WIDTH_DIM:
			return 6;

		case SPIN_ARROW_WIDTH_DIM:
			return 14;

		case SLIDER_FRAME_WIDTH_DIM:
			if (kind == HSLIDER_WIDGET || kind == VSLIDER_WIDGET)
				return 8;
			return 15;
		case SLIDER_HANDLE_WIDTH_DIM:
			if (kind == HSLIDER_WIDGET || kind == VSLIDER_WIDGET)
				return 6;
			return 15;
		case SLIDER_HANDLE_MIN_LENGTH_DIM:
			return 12;
		case SLIDER_TICKS_SPACE_DIM:
			return 8;

		case SCROLL_STEP_BUTTON_BORDER_DIM:
			return 0;
		case SCROLL_STEP_BUTTON_WIDTH_DIM:
			return 15;
		case SCROLL_SLIDER_BORDER_DIM:
			return 0;

		case SPLITTER_WIDTH_DIM:
			return 2;

		case MENU_ITEM_SPACING_DIM:
			return 2;
		case MENU_ITEM_ARROW_SPACING_DIM:
			return 2;
		case MENU_ITEM_ICON_SPACING_DIM:
			return 2;
		case MENU_ITEM_SEPARATOR_HEIGHT_DIM:
			return 1;
		case MENU_ITEM_CHECK_SIZE_DIM:
			return 8;
		case POPUP_MENU_VERTICAL_BORDER_DIM:
			return 4;

		case NOTEBOOK_TAB_OVERLAP_DIM:
			return 4;

		default:
			break;
	}
	return 0;
}
