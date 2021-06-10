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

#include "qsimplestyle.h"

#include <qapplication.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbutton.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qframe.h>
#include <qdialog.h>
#include <qpopupmenu.h>
#include <qtabbar.h>
#include <qprogressbar.h>

#define PRINT_INT(str, value)                   \
	printf (str "\n", value)
#define PRINT_RECT(str, rect)                   \
	printf (str " at %d x %d , %d x %d\n",        \
	        rect.x(), rect.y(), rect.width(), rect.height())
#define PRINT_INT_RECT(str, value, rect)        \
	printf (str " at %d x %d , %d x %d\n", value, \
	        rect.x(), rect.y(), rect.width(), rect.height())

QSimpleStyle::QSimpleStyle() : QCommonStyle()
{
	hover_subcontrol = SC_None;
}

QSimpleStyle::~QSimpleStyle()
{
}

/* We must implement highlight for ourselves. For simple widgets, like
   a button, we just need to force a redraw when the mouse gets over it
   or leaves it. But for more complex widgets, like scroll bars, we need
   to check mouse coordinates to see if it is over a step button or the
   handle. */
static const char *highlight_simple[] = {
	"QButton", "QHeader", "QScrollView", "QLineEdit",
	"QSplitterHandle", "QSplitter"
};
static const char *highlight_complex[] = {
	"QScrollBar", "QSpinWidget", "QComboBox"
};
#define highlight_simple_size (sizeof (highlight_simple) / sizeof (char*))
#define highlight_complex_size (sizeof (highlight_complex) / sizeof (char*))

void QSimpleStyle::polish (QWidget *widget)
{
	for (unsigned int i = 0; i < highlight_simple_size; i++) {
		if (widget->inherits (highlight_simple [i])) {
			widget->installEventFilter (this);
			break;
		}
	}
	for (unsigned int i = 0; i < highlight_complex_size; i++) {
		if (widget->inherits (highlight_complex [i])) {
			widget->setMouseTracking (true);
			widget->installEventFilter (this);
			break;
		}
	}
}

void QSimpleStyle::unPolish (QWidget *widget)
{
	for (unsigned int i = 0; i < highlight_simple_size; i++) {
		if (widget->inherits (highlight_simple [i])) {
			widget->removeEventFilter (this);
			break;
		}
	}
	for (unsigned int i = 0; i < highlight_complex_size; i++) {
		if (widget->inherits (highlight_complex [i])) {
			widget->setMouseTracking (false);
			widget->removeEventFilter (this);
			break;
		}
	}
}

bool QSimpleStyle::eventFilter (QObject *object, QEvent *event)
{
	QWidget *widget = static_cast <QWidget *> (object);
	switch (event->type()) {
		case QEvent::Enter:
			if (widget->hasMouseTracking() && (!object->inherits ("QComboBox") ||
			                                  ((QComboBox *) (object))->editable()))
				break;
		case QEvent::Leave:
			hover_subcontrol = SC_None;
			widget->repaint (true);
			break;
		case QEvent::MouseMove:
		{
			QPoint pos = ((QMouseEvent *) event)->pos();

			if (object->inherits ("QScrollBar")) {
				SubControl hover = querySubControl (CC_ScrollBar, widget, pos);
				if (hover != hover_subcontrol) {
					hover_subcontrol = hover;
					widget->repaint (false);
				}
			}
			else if (object->inherits ("QSpinWidget")) {
				SubControl hover = querySubControl (CC_SpinWidget, widget, pos);
				if (hover != hover_subcontrol) {
					hover_subcontrol = hover;
					widget->repaint (false);
				}
			}
			else if (object->inherits ("QComboBox") && ((QComboBox *) (object))->editable()) {
				SubControl hover = querySubControl (CC_ComboBox, widget, pos);
				if (hover != hover_subcontrol) {
					hover_subcontrol = hover;
					widget->repaint (false);
				}
			}
			break;
		}
		default:
			break;
	}
	return QCommonStyle::eventFilter (object, event);
}

void QSimpleStyle::drawPrimitive (PrimitiveElement primitive, QPainter *painter,
                                  const QRect &rect, const QColorGroup &colors,
                                  SFlags flags, const QStyleOption &opt) const
{
	setTarget (painter->device()->handle());

	switch (primitive) {
		case PE_ButtonTool:
			if (!(flags & (Style_On | Style_Down | Style_MouseOver)) ||
			    !(flags & Style_Enabled))
				break;
			paint (BUTTON_PAINT, TOOL_ITEM_WIDGET, flags, painter, colors, rect);
			break;
		case PE_ButtonCommand:
		case PE_ButtonDefault:
			paint (BUTTON_PAINT, BUTTON_WIDGET, flags, painter, colors, rect);
			break;

		case PE_Indicator:
		case PE_CheckMark:
			paint (CHECK_INDICATOR_PAINT, CHECK_WIDGET, flags, painter, colors, rect);
			break;
		case PE_CheckListIndicator:
			paint (CHECK_INDICATOR_PAINT, LISTVIEW_WIDGET, flags, painter, colors, rect);
			break;
		case PE_ExclusiveIndicator:
			paint (RADIO_INDICATOR_PAINT, RADIO_WIDGET, flags, painter, colors, rect);
			break;
		case PE_CheckListExclusiveIndicator:
			paint (RADIO_INDICATOR_PAINT, LISTVIEW_WIDGET, flags, painter, colors, rect);
			break;

		case PE_FocusRect:
			paint (FOCUS_PAINT, UNSPECIFIED_WIDGET, flags, painter, colors, rect);
			break;
#if 1
		// GTK+ 2.8 doesn't clip frames well...
		case PE_Panel:
		case PE_GroupBoxFrame:
		case PE_PanelGroupBox:
			if (opt.frameShadow() == QFrame::Sunken)
				flags |= Style_Sunken;
			else  // QFrame::Plain / QFrame::Raised
				flags |= Style_Raised;
			paint (FRAME_PAINT, UNSPECIFIED_WIDGET, flags, painter, colors, rect);
			break;
#endif
		case PE_PanelTabWidget:
		case PE_TabBarBase:
			paint (FRAME_PAINT, NOTEBOOK_WIDGET, flags, painter, colors, rect);
			break;

		case PE_PanelMenuBar:
			paint (BACKGROUND_PAINT, MENU_BAR_WIDGET, flags, painter, colors, rect);
			break;

		case PE_PanelPopup:
			paint (BACKGROUND_PAINT, MENU_WINDOW_WIDGET, flags, painter, colors, rect);
			break;

		case PE_PanelLineEdit:
		{
			// ugly check to see if this is a single or multiple line entry
			WidgetKind kind = rect.height() < 40 ? LINE_EDIT_WIDGET
			                                     : TEXT_VIEW_WIDGET;
			paint (ENTRY_PAINT, kind, flags, painter, colors, rect);
			break;
		}

		case PE_ArrowUp:
		case PE_ArrowDown:
		case PE_ArrowLeft:
		case PE_ArrowRight:
			Side side;
			if (primitive == PE_ArrowLeft)
				side = LeftSide;
			else if (primitive == PE_ArrowRight)
				side = RightSide;
			else if (primitive == PE_ArrowUp)
				side = TopSide;
			else  // if (primitive == PE_ArrowDown)
				side = BottomSide;
			paint (ARROW_PAINT, UNSPECIFIED_WIDGET, flags, painter, colors, rect, side);
			break;

		case PE_SpinWidgetUp:
		case PE_SpinWidgetDown:
		case PE_SpinWidgetPlus:
		case PE_SpinWidgetMinus:
		{
			Side side;
			if (primitive == PE_SpinWidgetUp || primitive == PE_SpinWidgetMinus)
				side = TopSide;
			else
				side = BottomSide;
			paint (BUTTON_PAINT, SPIN_WIDGET, flags, painter, colors, rect, side);

			int width = (rect.width() - 2) / 2;
			width -= (width % 2) - 1; /* force odd */
			int height = (width + 1) / 2;
			QRect arrow_rect (rect.x() + ((rect.width() - width) / 2),
			                  rect.y() + ((rect.height() - height) / 2) +
			                    ((side == TopSide) ? 1 : -1),
			                  width, height);
			paint (ARROW_PAINT, SPIN_WIDGET, flags, painter, colors, arrow_rect, side);
			break;
		}

		case PE_ScrollBarAddLine:
		case PE_ScrollBarSubLine:
		{
			bool horizontal = flags & Style_Horizontal;
			Side side;
			if (primitive == PE_ScrollBarSubLine) {
				if (horizontal)
					side = LeftSide;
				else
					side = TopSide;
			}
			else {
				if (horizontal)
					side = RightSide;
				else
					side = BottomSide;
			}
			WidgetKind kind = horizontal ? SCROLL_HBAR_WIDGET
			                  : SCROLL_VBAR_WIDGET;

			paint (BUTTON_PAINT, kind, flags, painter, colors, rect, side);

			int size = rect.width() / 2;
			QRect arrow_rect (rect.x() + (rect.width() - size)/2,
			                  rect.y() + (rect.height() - size)/2,
			                  size, size);
			if (flags & (Style_Down | Style_On))
				apply_style_shift (arrow_rect, kind, CHILD_DOWN_SHIFT_DIM);
			paint (ARROW_PAINT, kind, flags, painter, colors, arrow_rect, side);
			break;
		}
		case PE_ScrollBarSlider:
		{
			WidgetKind kind = flags & Style_Horizontal ? SCROLL_HBAR_WIDGET
			                  : SCROLL_VBAR_WIDGET;
			paint (SCROLL_HANDLE_PAINT, kind, flags, painter, colors, rect);
			break;
		}

		case PE_SizeGrip:
			paint (SIZE_GRIP_PAINT, UNSPECIFIED_WIDGET, flags, painter, colors, rect);
			break;

		case PE_Separator:
			// Separators are always horizontal and sometimes they don't specify it.
			// implementators should still honor vertical separators as we use it
			// for the combo box (unless you have just dismissed this call)
			flags |= Style_Horizontal;
			paint (SEPARATOR_PAINT, UNSPECIFIED_WIDGET, flags, painter, colors, rect);
			break;

		case PE_Splitter:
		{
			// Qt doesn't give good flags here
			flags |= Style_Enabled;
			const QWidget *widget = static_cast <const QWidget *> (painter->device());
			if (widget && widget->hasMouse())
				flags |= Style_MouseOver;
			paint (SPLITTER_PAINT, UNSPECIFIED_WIDGET, flags, painter, colors, rect);
			break;
		}

		case PE_HeaderSection:
			paint (HEADER_PAINT, LISTVIEW_WIDGET, flags, painter, colors, rect);
			break;
		case PE_HeaderArrow:
			paint (ARROW_PAINT, LISTVIEW_WIDGET, flags, painter, colors, rect);
			break;

		default:
			PRINT_INT_RECT ("unimplemented primitive %d", primitive, rect);
			QCommonStyle::drawPrimitive (primitive, painter, rect, colors, flags, opt);
			break;
	}
}

void QSimpleStyle::drawControl (ControlElement control, QPainter *painter,
                                const QWidget *widget, const QRect &rect,
                                const QColorGroup &colors, SFlags flags,
                                const QStyleOption &opt) const
{
	if (painter->device())
		setTarget (painter->device()->handle());

	if (widget->hasMouse())
		flags |= Style_MouseOver;

	switch (control) {
		case CE_PushButton:
			paint (BUTTON_PAINT, BUTTON_WIDGET, flags, painter, colors, rect);
			break;
		case CE_CheckBox:
		case CE_RadioButton:
		{
			WidgetKind kind = control == CE_CheckBox ? CHECK_WIDGET
			                                         : RADIO_WIDGET;
			if (flags & Style_MouseOver)
				paint (HIGHLIGHT_PAINT, kind, flags, painter, colors, rect);

			QRect indicator_rect = rect;
			apply_style_border (indicator_rect, kind, INDICATOR_BORDER_DIM);
			if (control == CE_CheckBox)
				paint (CHECK_INDICATOR_PAINT, kind, flags, painter, colors, indicator_rect);
			else
				paint (RADIO_INDICATOR_PAINT, kind, flags, painter, colors, indicator_rect);
			break;
		}
		case CE_CheckBoxLabel:
		case CE_RadioButtonLabel:
		{
			// we need to overload this just because we need to pad the label.
			// we can't just set that in subRect() because of the highlighted background.
			WidgetKind kind;
			if (control == CE_CheckBoxLabel)
				kind = CHECK_WIDGET;
			else
				kind = RADIO_WIDGET;

			if (flags & Style_MouseOver)
				paint (HIGHLIGHT_PAINT, kind, flags, painter, colors, rect);
#if 0
PRINT_RECT ("radio button label (before)", rect);
			QRect focus_rect (rect);
//			apply_style_border (focus_rect, kind, WIDGET_THICKNESS_DIM);
			QRect child_rect (focus_rect);
/*			apply_style_border (child_rect, kind, FOCUS_WIDTH_DIM);
			apply_style_border (child_rect, kind, FOCUS_BORDER_DIM);
			apply_style_border (child_rect, kind, CHILD_BORDER_DIM);*/
PRINT_RECT ("radio button label (after)", child_rect);
			const QButton *qbutton = static_cast <const QButton *> (widget);
			if (qbutton->isDown() || qbutton->isOn()) {
				apply_style_shift (child_rect, kind, CHILD_DOWN_SHIFT_DIM);
				apply_style_shift (focus_rect, kind, FOCUS_DOWN_SHIFT_DIM);
			}

			Qt::AlignmentFlags align = QApplication::reverseLayout() ? AlignRight : AlignLeft;
			drawItem (painter, child_rect, align | AlignVCenter | ShowPrefix, colors,
			          flags & Style_Enabled, qbutton->pixmap(), qbutton->text());

			if (flags & Style_HasFocus)
				paint (FOCUS_PAINT, kind, flags, painter, colors, focus_rect);
#else
			QCommonStyle::drawControl (control, painter, widget, rect, colors, flags, opt);
#endif
			break;
		}

		case CE_MenuBarEmptyArea:
			paint (BACKGROUND_PAINT, MENU_BAR_WIDGET, flags, painter, colors,
			       widget->rect());
			break;
		case CE_MenuBarItem:
		{
			drawControl (CE_MenuBarEmptyArea, painter, widget, rect, colors, flags, opt);
			bool highlight;
			highlight = (flags & Style_Down) && (flags & Style_Active) &&
			            (flags & Style_Enabled);
			if (highlight)
				paint (HIGHLIGHT_PAINT, MENU_ITEM_WIDGET, flags, painter, colors, rect);

			const QColor *color = highlight ? &colors.light() : &colors.text();
			QMenuItem *qitem = opt.menuItem();
			drawItem (painter, rect, AlignCenter | ShowPrefix,
			          colors, qitem->isEnabled(),
			          qitem->pixmap(), qitem->text(), -1, color);
			break;
		}
		case CE_PopupMenuItem:
		{
			// turn Style_Active into Style_MouseOver
			bool highlight= (flags & Style_Active) && (flags & Style_Enabled);
			if (flags & Style_Active)
				flags ^= Style_Active;
			if (flags & Style_MouseOver)
				flags ^= Style_MouseOver;
			if (highlight)
				flags |= Style_MouseOver;

			if (highlight)
				paint (HIGHLIGHT_PAINT, MENU_ITEM_WIDGET, flags, painter, colors, rect);

			QRect r (rect);
			add_hor_border (r, dimension (MENU_ITEM_SPACING_DIM, MENU_ITEM_WIDGET));

			const QPopupMenu *popup = static_cast <const QPopupMenu *> (widget);
			const QMenuItem *item = opt.menuItem();
			if (!item)
				break;

			if (item->isSeparator())
				paint (SEPARATOR_PAINT, MENU_ITEM_WIDGET, flags | Style_Horizontal,
				       painter, colors, r);
			else {
				QRect content (r);
				if (item->popup()) {
					QFontMetrics font_metrics (widget->font());
					int arrow_size = font_metrics.ascent() + font_metrics.descent();
					arrow_size -= 2 * dimension (WIDGET_THICKNESS_DIM, MENU_ITEM_WIDGET, TopSide);
					arrow_size = (int) (arrow_size * 0.8);
					int arrow_spacing = dimension (MENU_ITEM_ARROW_SPACING_DIM, MENU_ITEM_WIDGET);

					QRect arrow (r.x() + r.width() - arrow_size,
					             r.y() + ((r.height() - arrow_size) / 2),
					             arrow_size, arrow_size);
					paint (ARROW_PAINT, MENU_ITEM_WIDGET, flags, painter, colors, arrow, RightSide);

					r.setWidth (r.width() - arrow_spacing - arrow_size);
				}

				QRect icon (content), label (content);
				{
					int pics_size = 0;
					if (opt.maxIconWidth() || popup->isCheckable())
						pics_size = std::max (dimension (MENU_ITEM_CHECK_SIZE_DIM, MENU_ITEM_WIDGET),
						                      opt.maxIconWidth());
					icon.setRect (icon.x(), icon.y() + (icon.height() - pics_size)/2,
					              pics_size, pics_size);
				}
				if (icon.width() != 0)
					label.setX (label.x() + icon.width() +
					            dimension (MENU_ITEM_ICON_SPACING_DIM, MENU_ITEM_WIDGET));

				if (item->pixmap()) {
					const QPixmap *pixmap = item->pixmap();
					QRect r (0, 0, pixmap->width(), pixmap->height());
					r.moveCenter (icon.center());
					drawItem (painter, r, AlignCenter, colors, flags & Style_Enabled, pixmap, "");
				}
				else if (item->iconSet()) {
					QIconSet::Mode mode;
					if (!item->isEnabled())
						mode = QIconSet::Disabled;
					else if (flags & Style_Active)
						mode = QIconSet::Active;
					else
						mode = QIconSet::Normal;

					// TODO: we should indicate if the item is checked

					QPixmap pixmap (item->iconSet()->pixmap (QIconSet::Small, mode));
					QRect r (0, 0, pixmap.width(), pixmap.height());
					r.moveCenter (icon.center());
					painter->drawPixmap (r.topLeft(), pixmap);
				}

#if 0  // FIXME: how can we check if a menu item is checkable and is off?
				if (popup->isCheckable()) {
					SFlags subflags = flags;
					if (item->isChecked())
						subflags |= Style_On;
					paint (CHECK_INDICATOR_PAINT, MENU_ITEM_WIDGET, subflags, painter, colors, icon);
				}
#else
				else if (popup->isCheckable() && item->isChecked()) {
					SFlags subflags = flags | Style_On;
					paint (CHECK_INDICATOR_PAINT, MENU_ITEM_WIDGET, subflags, painter, colors, icon);
				}
#endif

				const QColor *color = highlight ? &colors.light() : &colors.text();
				// draw accelerator text on the right
				QString text = item->text();
				int t = text.find ('\t');
				drawItem (painter, label, AlignLeft | AlignVCenter | ShowPrefix,
				          colors, flags & Style_Enabled, NULL, text, t, color);
				if (t != -1) {
					text.remove (0, t+1);
					drawItem (painter, label, AlignRight | AlignVCenter,
					          colors, flags & Style_Enabled, NULL, text, -1, color);
				}
			}
			break;
		}

		case CE_PopupMenuScroller:
		case CE_PopupMenuHorizontalExtra:
		case CE_PopupMenuVerticalExtra:
PRINT_RECT ("draw popup menu stuff to implement", rect);
			break;

		case CE_TabBarTab:
		{
			PRINT_RECT ("draw tab", rect);
			Side side = TopSide;
			const QTabBar *tab = static_cast <const QTabBar *> (widget);
			if (tab && (tab->shape() == QTabBar::RoundedBelow ||
			            tab->shape() == QTabBar::TriangularBelow))
				side = BottomSide;
//			painter->setClipRect (rect);
			paint (HEADER_PAINT, NOTEBOOK_WIDGET, flags, painter, colors, rect, side);
			break;
		}

		case CE_ProgressBarGroove:
			paint (FRAME_PAINT, PROGRESS_BAR_WIDGET, flags, painter, colors, rect);
			break;
		case CE_ProgressBarContents:
		{
			const QProgressBar *bar = static_cast <const QProgressBar *> (widget);
			if (bar->progress() > 0) {
				const int width = (bar->progress() * rect.width()) / bar->totalSteps();
				QRect r (rect);
				r.setWidth (width);
				add_border (r, dimension (WIDGET_THICKNESS_DIM, PROGRESS_BAR_WIDGET));
				paint (PROGRESS_BAR_PAINT, PROGRESS_BAR_WIDGET, flags, painter, colors, r);
			}
			break;
		}

		default:
			PRINT_INT_RECT ("unimplemented control %d", control, rect);
			QCommonStyle::drawControl (control, painter, widget, rect, colors, flags, opt);
			break;
	}
}

void QSimpleStyle::drawControlMask (ControlElement element, QPainter *painter,
                                 const QWidget *widget, const QRect &rect,
                                 const QStyleOption &opt) const
{
	QCommonStyle::drawControlMask (element, painter, widget, rect, opt);
}

void QSimpleStyle::drawComplexControl (ComplexControl control, QPainter *painter,
                                    const QWidget *widget, const QRect &rect,
                                    const QColorGroup &colors, SFlags flags,
                                    SCFlags subControl, SCFlags activeSubControl,
                                    const QStyleOption &opt) const
{
	setTarget (painter->device()->handle());

	if (widget->hasMouse())
		flags |= Style_MouseOver;

	switch (control) {
		case CC_ToolButton:
			drawPrimitive (PE_ButtonTool, painter, rect, colors, flags);
			break;

		case CC_ScrollBar:
		{
			const QScrollBar *qscroll = static_cast <const QScrollBar *> (widget);
			bool horizontal = qscroll->orientation() == Qt::Horizontal;
			WidgetKind kind = horizontal ? SCROLL_HBAR_WIDGET : SCROLL_VBAR_WIDGET;
			if (flags & Style_MouseOver)
				flags ^= Style_MouseOver;
			bool dragging = qscroll->draggingSlider();

			if (subControl & SC_ScrollBarSlider) {
				QRect subrect = querySubControlMetrics (CC_ScrollBar, widget, SC_ScrollBarGroove, opt);
				// we want to clip this because we don't want the top/bottom corners to get drawn
				painter->save();
				painter->setClipRect (subrect);
				paint (SCROLL_GROOVE_PAINT, kind, flags, painter, colors, rect);
				painter->restore();

				subrect = querySubControlMetrics (CC_ScrollBar, widget, SC_ScrollBarSlider, opt);
				SFlags subflags = flags;
				if (hover_subcontrol == SC_ScrollBarSlider || dragging)
					subflags |= Style_MouseOver;
				paint (SCROLL_HANDLE_PAINT, kind, subflags, painter, colors, subrect);
			}

			// TODO: suppport secondary step buttons (if style sets)
			if (subControl & SC_ScrollBarSubLine) {
				SFlags subflags = flags;
				if (activeSubControl & SC_ScrollBarSubLine)
					subflags |= Style_Down;
/*			if (qscroll->value() == qscroll->minValue())
					subflags ^= Style_Enabled;*/
				if (horizontal)
					subflags |= Style_Horizontal;
				if (hover_subcontrol == SC_ScrollBarSubLine && !dragging)
					subflags |= Style_MouseOver;

				QRect subrect = querySubControlMetrics (CC_ScrollBar, widget, SC_ScrollBarSubLine, opt);
				drawPrimitive (PE_ScrollBarSubLine, painter, subrect, colors, subflags, opt);
			}
			if (subControl & SC_ScrollBarAddLine) {
				SFlags subflags = flags;
				if (activeSubControl & SC_ScrollBarAddLine)
					subflags |= Style_Down;
/*			if (qscroll->value() == qscroll->maxValue())
					subflags ^= Style_Enabled;*/
				if (horizontal)
					subflags |= Style_Horizontal;
				if (hover_subcontrol == SC_ScrollBarAddLine && !dragging)
					subflags |= Style_MouseOver;

				QRect subrect = querySubControlMetrics (CC_ScrollBar, widget, SC_ScrollBarAddLine, opt);
				drawPrimitive (PE_ScrollBarAddLine, painter, subrect, colors, subflags, opt);
			}
			break;
		}

		case CC_SpinWidget:
		{
			const QSpinWidget *qspin = static_cast <const QSpinWidget *> (widget);
			WidgetKind kind = SPIN_WIDGET;
			if (flags & Style_MouseOver)
				flags ^= Style_MouseOver;

			if (subControl & (SC_SpinWidgetFrame | SC_SpinWidgetEditField)) {
				QRect subrect = querySubControlMetrics (CC_SpinWidget, widget,
				                                        SC_SpinWidgetFrame, opt);
				paint (ENTRY_PAINT, kind, flags, painter, colors, subrect);
			}

			if (subControl & SC_SpinWidgetButtonField) {
				QRect subrect = querySubControlMetrics (CC_SpinWidget, widget,
				                                        SC_SpinWidgetButtonField, opt);
				paint (SPIN_BUTTONS_BOX_PAINT, kind, flags, painter, colors, subrect);
				// draw also the buttons, when you draw the box
				subControl |= SC_SpinWidgetUp;
				subControl |= SC_SpinWidgetDown;
			}

			if (subControl & SC_SpinWidgetUp) {
				QRect subrect = querySubControlMetrics (CC_SpinWidget, widget,
				                                        SC_SpinWidgetUp, opt);
				SFlags subflags = flags;
				if (activeSubControl & SC_SpinWidgetUp)
					subflags |= Style_Down;
				if (!qspin->isUpEnabled() && (flags & Style_Enabled))
					subflags ^= Style_Enabled;
				if (hover_subcontrol == SC_SpinWidgetUp)
					subflags |= Style_MouseOver;
				drawPrimitive (PE_SpinWidgetUp, painter, subrect, colors, subflags, opt);
			}
			if (subControl & SC_SpinWidgetDown) {
				QRect subrect = querySubControlMetrics (CC_SpinWidget, widget,
				                                        SC_SpinWidgetDown, opt);
				SFlags subflags = flags;
				if (activeSubControl & SC_SpinWidgetDown)
					subflags |= Style_Down;
				if (!qspin->isDownEnabled() && (flags & Style_Enabled))
					subflags ^= Style_Enabled;
				if (hover_subcontrol == SC_SpinWidgetDown)
					subflags |= Style_MouseOver;
				drawPrimitive (PE_SpinWidgetDown, painter, subrect, colors, subflags, opt);
			}
			break;
		}

		case CC_ComboBox:
		{
			const QComboBox *qcombo = static_cast <const QComboBox *> (widget);
			// TODO: how can one check if the box is being pressed?
			if (qcombo->editable()) {
				WidgetKind kind = COMBO_ENTRY_WIDGET;
				if (flags & Style_MouseOver)
					flags ^= Style_MouseOver;

				QRect arrow_rect = querySubControlMetrics (CC_ComboBox, widget, SC_ComboBoxArrow, opt);
				if (subControl & SC_ComboBoxFrame) {
					QRect subrect (rect);
					subrect.setWidth (subrect.width() - arrow_rect.width());
					paint (ENTRY_PAINT, LINE_EDIT_WIDGET, flags, painter, colors, subrect);
				}

				if (subControl & SC_ComboBoxArrow) {
					SFlags subflags = flags;
					if (hover_subcontrol == SC_ComboBoxArrow)
						subflags |= Style_MouseOver;

					QRect subrect = querySubControlMetrics (CC_ComboBox, widget, SC_ComboBoxArrow, opt);
					paint (BUTTON_PAINT, kind, subflags, painter, colors, subrect);

					apply_style_border (arrow_rect, kind, COMBO_ARROW_BORDER_DIM);
					paint (ARROW_PAINT, kind, subflags, painter, colors, arrow_rect, BottomSide);
				}
			}
			else {
				WidgetKind kind = COMBO_WIDGET;
				if (subControl & SC_ComboBoxFrame) {
					paint (BUTTON_PAINT, kind, flags, painter, colors, rect);
				}

				if (subControl & SC_ComboBoxArrow) {
					QRect subrect = querySubControlMetrics (CC_ComboBox, widget, SC_ComboBoxArrow, opt);

					QRect separator_rect = subrect;
					separator_rect.setWidth (2);
					paint (SEPARATOR_PAINT, kind, flags, painter, colors, separator_rect);

					QRect arrow_rect = subrect;
					arrow_rect.setX (arrow_rect.x() + 2 +
					    dimension (COMBO_ARROW_BORDER_DIM, kind, LeftSide));
					arrow_rect.setWidth (dimension (COMBO_ARROW_WIDTH_DIM, kind));
					paint (ARROW_PAINT, kind, flags, painter, colors, arrow_rect, BottomSide);
				}

				if ((subControl & SC_ComboBoxEditField) && (flags & Style_HasFocus)) {
					QRect subrect = rect;
					apply_style_border (subrect, kind, FOCUS_BORDER_DIM);
					apply_style_border (subrect, kind, FOCUS_WIDTH_DIM);
					paint (FOCUS_PAINT, kind, flags, painter, colors, subrect);
				}
			}
			break;
		}

		case CC_Slider:
		{
			const QSlider *qslider = static_cast <const QSlider *> (widget);
			bool horizontal = qslider->orientation() == Qt::Horizontal;
			WidgetKind kind = horizontal ? HSLIDER_WIDGET : VSLIDER_WIDGET;

			if (subControl & SC_SliderGroove) {
				QRect subrect = querySubControlMetrics (CC_Slider, widget,
				                                        SC_SliderGroove, opt);
				paint (SCROLL_GROOVE_PAINT, kind, flags, painter, colors, subrect);
			}

			if (subControl & SC_SliderHandle) {
				QRect subrect = querySubControlMetrics (CC_Slider, widget,
				                                        SC_SliderHandle, opt);
				paint (SCROLL_HANDLE_PAINT, kind, flags, painter, colors, subrect);
			}

			if (subControl & SC_SliderTickmarks) {
				// let Qt draw this
				QCommonStyle::drawComplexControl (control, painter, widget, rect,
				                                  colors, flags, subControl,
				                                  activeSubControl, opt);
			}
			break;
		}


		default:
			PRINT_INT_RECT ("unimplemented complex control %d", control, rect);
			QCommonStyle::drawComplexControl (control, painter, widget, rect,
			                                  colors, flags, subControl,
			                                  activeSubControl, opt);
			break;
	}
}

void QSimpleStyle::drawComplexControlMask (ComplexControl control, QPainter *painter,
                                        const QWidget *widget, const QRect &rect,
                                        const QStyleOption &opt) const
{
	QCommonStyle::drawComplexControlMask (control, painter, widget, rect, opt);
}

int QSimpleStyle::pixelMetric (PixelMetric metric, const QWidget *widget) const
{
	switch (metric) {
		case PM_ButtonMargin:
			return dimension (WIDGET_THICKNESS_DIM, BUTTON_WIDGET) +
			       dimension (CHILD_BORDER_DIM, BUTTON_WIDGET) +
			       dimension (FOCUS_BORDER_DIM, BUTTON_WIDGET) +
			       dimension (FOCUS_WIDTH_DIM, BUTTON_WIDGET);
	  	case PM_ButtonDefaultIndicator:
	  		return 0;
	  	case PM_MenuButtonIndicator:
	  		return widget->height() / 2;
		case PM_ButtonShiftHorizontal:
			return dimension (CHILD_DOWN_SHIFT_DIM, BUTTON_WIDGET);
		case PM_ButtonShiftVertical:
			return dimension (CHILD_DOWN_SHIFT_DIM, BUTTON_WIDGET);

		case PM_IndicatorWidth:
			return dimension (INDICATOR_WIDTH_DIM, CHECK_WIDGET);
		case PM_ExclusiveIndicatorWidth:
			return dimension (INDICATOR_WIDTH_DIM, RADIO_WIDGET);
		case PM_IndicatorHeight:
			return dimension (INDICATOR_HEIGHT_DIM, CHECK_WIDGET);
		case PM_ExclusiveIndicatorHeight:
			return dimension (INDICATOR_HEIGHT_DIM, RADIO_WIDGET);

		case PM_DefaultFrameWidth:
		case PM_SpinBoxFrameWidth:
			// this is used for Line Edits (GtkEntries) too... :(
			return 3;

		case PM_MenuBarFrameWidth:
			return dimension (CHILD_BORDER_DIM, MENU_BAR_WIDGET) + 1;
		case PM_MenuBarItemSpacing:
			return dimension (MENU_ITEM_SPACING_DIM, MENU_ITEM_WIDGET);
		case PM_ToolBarItemSpacing:
			return 1;
		case PM_PopupMenuFrameHorizontalExtra:
			return 0;
		case PM_PopupMenuFrameVerticalExtra:
			return dimension (POPUP_MENU_VERTICAL_BORDER_DIM, MENU_WINDOW_WIDGET) * 2;

		case PM_ScrollBarExtent:
			return dimension (SLIDER_FRAME_WIDTH_DIM, SCROLL_VBAR_WIDGET);
		case PM_ScrollBarSliderMin:
			return dimension (SLIDER_HANDLE_MIN_LENGTH_DIM, SCROLL_VBAR_WIDGET);
		case PM_MaximumDragDistance:
			return -1;

		case PM_SliderThickness:
			return dimension (SLIDER_TICKS_SPACE_DIM, HSLIDER_WIDGET) +
			       dimension (SLIDER_FRAME_WIDTH_DIM, HSLIDER_WIDGET);
		case PM_SliderControlThickness:
			return dimension (SLIDER_FRAME_WIDTH_DIM, HSLIDER_WIDGET);
		case PM_SliderLength:
			return dimension (SLIDER_HANDLE_WIDTH_DIM, HSLIDER_WIDGET);
		case PM_SliderTickmarkOffset:
			return dimension (SLIDER_TICKS_SPACE_DIM, HSLIDER_WIDGET);

		case PM_SplitterWidth:
			return dimension (SPLITTER_WIDTH_DIM, UNSPECIFIED_WIDGET);

		// TODO: check values
		case PM_TabBarTabShiftHorizontal:
		case PM_TabBarTabShiftVertical:
			return 2;

		case PM_TabBarTabOverlap:
			return 0;
			return dimension (NOTEBOOK_TAB_OVERLAP_DIM, NOTEBOOK_WIDGET);

/*
		case PM_TabBarTabHSpace:
			return 20;
		case PM_TabBarTabVSpace:
			return 10;
		case PM_TabBarBaseHeight:
		case PM_TabBarTabShiftHorizontal:
		case PM_TabBarTabShiftVertical:
			return 0;
		case PM_TabBarScrollButtonWidth:
			return 8;
*/
		default:
			PRINT_INT ("unimplemented metric %d", metric);
			return QCommonStyle::pixelMetric (metric, widget);
	}
}

QSize QSimpleStyle::sizeFromContents (ContentsType contents, const QWidget *widget,
                                      const QSize &contentsSize, const QStyleOption &opt) const
{
	QSize size = contentsSize;

	switch (contents) {
		case CT_PushButton:
		case CT_ComboBox:
//		case CT_RadioButton:
		{
			WidgetKind kind;
			if (contents == CT_PushButton)
				kind = BUTTON_WIDGET;
			else {
				if (static_cast <const QComboBox *> (widget)->editable())
					kind = COMBO_ENTRY_WIDGET;
				else
					kind = COMBO_WIDGET;
			}
			/* consider every button as can default */
			apply_style_size (size, kind, BUTTON_BORDER_DIM);
			apply_style_size (size, kind, WIDGET_THICKNESS_DIM);
			apply_style_size (size, kind, FOCUS_BORDER_DIM);
			apply_style_size (size, kind, FOCUS_WIDTH_DIM);
			apply_style_size (size, kind, CHILD_BORDER_DIM);
			if (contents == CT_ComboBox)
				size += QSize (dimension (COMBO_ARROW_BORDER_DIM, kind) * 2 +
				               dimension (COMBO_ARROW_WIDTH_DIM, kind) + 2, 0);
			break;
		}
		case CT_PopupMenuItem:
		{
			const QPopupMenu *popup = static_cast <const QPopupMenu *> (widget);
			const QMenuItem *item = opt.menuItem();
			apply_style_size (size, MENU_ITEM_WIDGET, MENU_ITEM_SPACING_DIM);
			if (item->isSeparator())
				size += QSize (0, dimension (MENU_ITEM_SEPARATOR_HEIGHT_DIM, MENU_ITEM_WIDGET));
			else {
				int pics_size = 0;
				if (opt.maxIconWidth() || popup->isCheckable()) {
					pics_size = std::max (dimension (MENU_ITEM_CHECK_SIZE_DIM, MENU_ITEM_WIDGET),
					                      opt.maxIconWidth());
					pics_size += dimension (MENU_ITEM_ICON_SPACING_DIM, MENU_ITEM_WIDGET);
				}
				size += QSize (pics_size, 0);
				if (item->popup())
					// FIXME: add the arrow size too?
					size += QSize (dimension (MENU_ITEM_ARROW_SPACING_DIM, MENU_ITEM_WIDGET), 0);
			}
			break;
		}
		default:
			size = QCommonStyle::sizeFromContents (contents, widget, contentsSize, opt);
			break;
	}
	return size;
}

QRect QSimpleStyle::subRect (SubRect subrect, const QWidget *widget) const
{
	QRect rect = widget->rect();

	switch (subrect) {
		case SR_PushButtonFocusRect:
		{
			WidgetKind kind = BUTTON_WIDGET;
			apply_style_border (rect, kind, BUTTON_BORDER_DIM);
			apply_style_border (rect, kind, WIDGET_THICKNESS_DIM);

			const QButton *qbutton = dynamic_cast <const QButton *> (widget);
			if (qbutton->isDown() || qbutton->isOn())
				apply_style_border (rect, kind, FOCUS_DOWN_SHIFT_DIM);
			break;
		}
		case SR_PushButtonContents:
		{
			WidgetKind kind = BUTTON_WIDGET;
			apply_style_border (rect, kind, BUTTON_BORDER_DIM);
			apply_style_border (rect, kind, WIDGET_THICKNESS_DIM);
			apply_style_border (rect, kind, FOCUS_WIDTH_DIM);
			apply_style_border (rect, kind, FOCUS_BORDER_DIM);
			apply_style_border (rect, kind, CHILD_BORDER_DIM);

			const QButton *qbutton = dynamic_cast <const QButton *> (widget);
			if (qbutton->isDown() || qbutton->isOn())
				apply_style_border (rect, kind, CHILD_DOWN_SHIFT_DIM);
			break;
		}
		case SR_CheckBoxIndicator:
		case SR_RadioButtonIndicator:
		{
			// overloading to make some spacing for indicator for highlighting
			WidgetKind kind = subrect == SR_CheckBoxIndicator ?
			                  CHECK_WIDGET : RADIO_WIDGET;
			int width = dimension (INDICATOR_BORDER_DIM, kind, LeftSide) + 
			            dimension (INDICATOR_WIDTH_DIM, kind) +
			            dimension (INDICATOR_BORDER_DIM, kind, RightSide);
			rect = QRect (0, 0, width, rect.height());
			break;
		}
		case SR_CheckBoxContents:
		case SR_RadioButtonContents:
		{
			QRect indicator = subRect (SR_CheckBoxIndicator, widget);
			rect = QRect (indicator.width(), 0, rect.width() - indicator.width(),
			              rect.height());
			break;
		}
		case SR_CheckBoxFocusRect:
		case SR_RadioButtonFocusRect:
			rect = QCommonStyle::subRect (subrect, widget);
			rect.setX (rect.x() + dimension (INDICATOR_BORDER_DIM, CHECK_WIDGET) + 1);
			break;
		case SR_SliderFocusRect:
			// TODO
			break;
		default:
			rect = QCommonStyle::subRect (subrect, widget);
			break;
	}
	return rect;
}

QRect QSimpleStyle::querySubControlMetrics (ComplexControl control,
	const QWidget *widget, SubControl subcontrol, const QStyleOption &opt) const
{
	QRect rect = widget->rect();

	switch (control) {
		case CC_ScrollBar:
		{
			bool horizontal = static_cast <const QScrollBar *> (widget)->orientation()
			                  == Qt::Horizontal;
			WidgetKind kind = horizontal ? SCROLL_HBAR_WIDGET : SCROLL_VBAR_WIDGET;

			// just to add some border to the buttons and the slider
			rect = QCommonStyle::querySubControlMetrics (control, widget, subcontrol, opt);

			if (subcontrol == SC_ScrollBarAddLine || subcontrol == SC_ScrollBarSubLine)
				apply_style_border (rect, kind, SCROLL_STEP_BUTTON_BORDER_DIM);
			else if (subcontrol == SC_ScrollBarSlider)
				apply_style_border (rect, kind, SCROLL_SLIDER_BORDER_DIM);
			break;
		}

		case CC_SpinWidget:
		{
			WidgetKind kind = SPIN_WIDGET;
			switch (subcontrol) {
				case SC_SpinWidgetButtonField:
				{
					int width = dimension (SPIN_ARROW_WIDTH_DIM, kind) +
					            dimension (WIDGET_THICKNESS_DIM, kind)*2;
					rect.moveLeft (rect.right() - width);
					rect.setWidth (width);
					break;
				}
				case SC_SpinWidgetUp:
				case SC_SpinWidgetDown:
					rect = querySubControlMetrics (control, widget, SC_SpinWidgetButtonField, opt);
					if (subcontrol == SC_SpinWidgetDown) {
						int y = rect.y() + rect.height()/2;
						rect.setHeight (rect.height() - (y - rect.y()));
						rect.moveTop (y);
					}
					else
						rect.setHeight (rect.height() / 2);
					break;
				case SC_SpinWidgetEditField:
				case SC_SpinWidgetFrame:
				{
					QRect buttons = querySubControlMetrics (control, widget,
					                    SC_SpinWidgetButtonField, opt);
					rect.setWidth (rect.width() - buttons.width() - 1);
					if (subcontrol == SC_SpinWidgetEditField)
						add_border (rect, pixelMetric (PM_SpinBoxFrameWidth, widget));
					break;
				}
				default:
					break;
			}
			break;
		}

		case CC_ComboBox:
		{
			const QComboBox *qcombo = static_cast <const QComboBox *> (widget);
			bool editable = qcombo->editable();
			WidgetKind kind = editable ? COMBO_ENTRY_WIDGET : COMBO_WIDGET;
			switch (subcontrol) {
				case SC_ComboBoxEditField:
				{
					if (editable)
						add_border (rect, pixelMetric (PM_DefaultFrameWidth, widget));
					else {
						apply_style_border (rect, kind, WIDGET_THICKNESS_DIM);
						apply_style_border (rect, kind, FOCUS_BORDER_DIM);
						apply_style_border (rect, kind, FOCUS_WIDTH_DIM);
						apply_style_border (rect, kind, CHILD_BORDER_DIM);
					}
					QRect arrow_rect;
					arrow_rect = querySubControlMetrics (CC_ComboBox, widget, SC_ComboBoxArrow, opt);
					rect.setWidth (rect.width() - arrow_rect.width());
					break;
				}
				case SC_ComboBoxFrame:
					break;
				case SC_ComboBoxArrow:
				{
					if (!editable) {
						apply_style_border (rect, kind, WIDGET_THICKNESS_DIM);
						apply_style_border (rect, kind, CHILD_BORDER_DIM);
					}
					int w = dimension (COMBO_ARROW_BORDER_DIM, kind, LeftSide) +
					        dimension (COMBO_ARROW_WIDTH_DIM, kind) +
					        dimension (COMBO_ARROW_BORDER_DIM, kind, RightSide) + 2;
					rect.setX (rect.x() + rect.width() - w);
					rect.setWidth (w);
					break;
				}
				default:
					rect = QCommonStyle::querySubControlMetrics (control, widget, subcontrol, opt);
					break;
			}
			break;
		}
		default:
			rect = QCommonStyle::querySubControlMetrics (control, widget, subcontrol, opt);
			break;
	}
	return rect;
}

QStyle::SubControl QSimpleStyle::querySubControl (ComplexControl control,
	const QWidget *widget, const QPoint &pos, const QStyleOption &opt) const
{
	switch (control) {
		case CC_ComboBox:
			if (querySubControlMetrics (control, widget, SC_ComboBoxArrow, opt).contains (pos))
				return SC_ComboBoxArrow;
			return SC_None;
		case CC_SpinWidget:
			if (querySubControlMetrics (control, widget, SC_SpinWidgetUp, opt).contains (pos))
				return SC_SpinWidgetUp;
			if (querySubControlMetrics (control, widget, SC_SpinWidgetDown, opt).contains (pos))
				return SC_SpinWidgetDown;
			return SC_None;
		default:
			return QCommonStyle::querySubControl (control, widget, pos, opt);
	}
}

// Aiders
void QSimpleStyle::apply_style_border (QRect &rect, WidgetKind kind,
                                       Dimension dim) const
{
	int left  = dimension (dim, kind, LeftSide);
	int right = dimension (dim, kind, RightSide);
	int up    = dimension (dim, kind, TopSide);
	int down  = dimension (dim, kind, BottomSide);
	rect.setRect (rect.x() + left, rect.y() + up,
	              rect.width() - left - right,
	              rect.height() - up - down);
}

void QSimpleStyle::apply_style_size (QSize &size, WidgetKind kind,
                                     Dimension dim) const
{
	int left  = dimension (dim, kind, LeftSide);
	int right = dimension (dim, kind, RightSide);
	int up    = dimension (dim, kind, TopSide);
	int down  = dimension (dim, kind, BottomSide);
	size += QSize (left + right, up + down);
}

void QSimpleStyle::apply_style_shift (QRect &rect, WidgetKind kind,
                                      Dimension dim) const
{
	int left  = dimension (dim, kind, LeftSide);
	int up    = dimension (dim, kind, TopSide);
	rect.setRect (rect.x() + left, rect.y() + up, rect.width(), rect.height());
}
