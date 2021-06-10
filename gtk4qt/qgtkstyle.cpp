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

/* Widgets drawing technic:
   What we do here to draw the GTK+ widgets is to take advantage of the fact
   that both QPixmap and GdkPixmap are just wrappers for a Xlib Pixmap. So,
   we request the QPixmap that the QPainter is drawing upon and with its handle
   we ceate a GdkPixmap that we feed to GtkStyle paint functions.

   Even though gtk_paint_*() asks for a GdkWindow, any GdkDrawable should do.  */

// There seem to be some name collisions here
#include <gtk/gtk.h>

#include <qpalette.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qcombobox.h>

#include "qgtkstyle.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#define EXPAND_RECT(rect) rect.x(), rect.y(), rect.width(), rect.height()

QGtkStyle::QGtkStyle() : QSimpleStyle()
{
	// prepare environment
	gtk_init (NULL, NULL);

	GdkDisplay *display = gdk_x11_foreign_new_xdisplay (QPaintDevice::x11AppDisplay(), true);
	gdk_display_manager_set_default_display (gdk_display_manager_get(), display);

	Visual *xvisual = (Visual *) QPaintDevice::x11AppVisual();
	GdkVisual *visual = gdkx_visual_get (xvisual->visualid);
	colormap = gdk_x11_colormap_foreign_new (visual, QPaintDevice::x11AppColormap());

	// current target
	drawable = NULL;

	// create base widgets
	getWidget (GTK_TYPE_WINDOW);
	getWidget (GTK_TYPE_FIXED);
}

QGtkStyle::~QGtkStyle()
{
	setTarget (0);
	// this will destroy the all thing
	gtk_widget_destroy (window_widget);
}

void QGtkStyle::polish (QApplication *app)
{
	PangoFontDescription *font = window_widget->style->font_desc;
	const char *family = pango_font_description_get_family (font);
	int pixel_height   = pango_font_description_get_size (font) / PANGO_SCALE;
	PangoStyle pango_style = pango_font_description_get_style (font);
	PangoWeight	pango_weight = pango_font_description_get_weight (font);
//	PangoStretch pango_stretch = pango_font_description_get_stretch (font);
	// FIXME: maybe we can just weight as equal since Qt follows CSS (and maybe Pango?)
	int weight;
	if (pango_weight <= PANGO_WEIGHT_LIGHT)
		weight = QFont::Light;
	else if (pango_weight <= PANGO_WEIGHT_NORMAL)
		weight = QFont::Normal;
	else if (pango_weight <= PANGO_WEIGHT_BOLD)
		weight = QFont::Bold;
	else
		weight = QFont::Black;
	bool italic = pango_style == PANGO_STYLE_ITALIC;
// TODO: support stretch

	app->setFont (QFont (family, pixel_height, weight, italic));

	QCommonStyle::polish (app);
}

void QGtkStyle::polish (QPalette &pal)
{
printf ("polish palette\n");
	/* Importing the GTK+ palette to Qt is important because labels and other
	   things will still be drawn by it, and it will make user-made widgets
	   look better. */
	// TODO: Qt surely supports a prelight color ... but where?
	GtkStyle *style = window_widget->style;

#define COLOR(clr, state) (QBrush (toQtColor (style->clr [state])))
#define COLOR_N(clr) COLOR (clr, GTK_STATE_NORMAL)
#define COLOR_I(clr) COLOR (clr, GTK_STATE_INSENSITIVE)

	// foregound, button, light, dark, mid, text, bright_text, base, background
	QColorGroup normal (COLOR_N (fg), COLOR_N (bg), COLOR_N (light),
		COLOR_N (dark),	COLOR_N (mid), COLOR_N (text), COLOR (text, GTK_STATE_PRELIGHT),
		COLOR_N (base),	COLOR_N (bg));

	QColorGroup disabled (COLOR_I (fg), COLOR_N (bg), COLOR_I (light),
		COLOR_I (dark), COLOR_I (mid), COLOR_I (text), COLOR_I (text_aa) /* other? */,
		COLOR_I (base),	COLOR_N (bg));

	// some extra definitions
//	normal.setBrush (QColorGroup::ButtonText, COLOR ( ));  // same as text
//	normal.setBrush (QColorGroup::Shadow, COLOR ( ));  // black
	normal.setBrush (QColorGroup::Highlight, COLOR (bg, GTK_STATE_SELECTED));
	normal.setBrush (QColorGroup::HighlightedText, COLOR (fg, GTK_STATE_SELECTED));
// GTK 2.10 defines this:
//	normal.setBrush (QColorGroup::Link, COLOR ( ));
//	normal.setBrush (QColorGroup::LinkVisited, COLOR ( ));

	if (style->bg_pixmap [GTK_STATE_NORMAL]) {
		QBrush bg_brush (COLOR_N (bg));
		bg_brush.setPixmap (toQtPixmap (style->bg_pixmap [GTK_STATE_NORMAL],
		                                style->bg_gc [GTK_STATE_NORMAL]));
		normal.setBrush (QColorGroup::Background, bg_brush);
	}
	if (style->bg_pixmap [GTK_STATE_INSENSITIVE]) {
		QBrush bg_brush (COLOR_I (bg));
		bg_brush.setPixmap (toQtPixmap (style->bg_pixmap [GTK_STATE_INSENSITIVE],
		                                style->bg_gc [GTK_STATE_INSENSITIVE]));
		normal.setBrush (QColorGroup::Background, bg_brush);
	}

#undef COLOR_I
#undef COLOR_N
#undef COLOR

	// active and inactive refers to if the window has focus or not. :)
	// not badly thought actually... making an unfocused window a bit darker...
	pal.setActive (normal);
	pal.setInactive (normal);
	pal.setDisabled (disabled);

	QCommonStyle::polish (pal);
}

/* TODO: we may also want to disable background color filling with
   setBackgroundMode (Qt::NoBackground) for widgets where we draw
   the entire thing.

   We should also set the palette for each widget. */

#if 0
void QGtkStyle::polish (QWidget *widget)
{
	/* Make frames following Gnome HIG */
	QGroupBox *group_box = dynamic_cast <QGroupBox *> (widget);
	if (group_box) {
		QString title = "<b>" + group_box->title() + "</b>";
		group_box->setTitle (title);
	}
}
#endif

void QGtkStyle::setTarget (Qt::HANDLE handle) const
{
	QGtkStyle *pThis = const_cast <QGtkStyle *> (this);
	if (drawable) {
		g_object_unref (G_OBJECT (pThis->drawable));
		pThis->drawable = NULL;
	}
	if (handle) {
		pThis->drawable = GDK_DRAWABLE (gdk_pixmap_foreign_new (handle));
		gdk_drawable_set_colormap (pThis->drawable, colormap);
	}
}

void QGtkStyle::paint (Paint paint_what, WidgetKind kind, SFlags flags,
	                     QPainter *painter, const QColorGroup &colors,
	                     const QRect &_rect, Side side) const
{
	// QPainter may have set translations or some such...
	const QRect rect = painter->xForm (_rect);

	if (painter->hasClipping()) {
		QMemArray <QRect> clips = painter->clipRegion().rects();
		for (unsigned int i = 0; i < clips.size(); i++) {
			GdkRectangle area = toGdkRect (clips[i]);
			paintClip (paint_what, kind, flags, rect, &area, side);
		}
	}
	else
		paintClip (paint_what, kind, flags, rect, NULL, side);
}

void QGtkStyle::paintClip (Paint paint_what, WidgetKind kind, SFlags flags,
                           const QRect &rect, GdkRectangle *area, Side side) const
{
	GtkWidget *widget = getWidget (kind);
	const char *detail = NULL;

	GtkStateType state;
	GtkShadowType shadow;
	toGtkFlags (flags, &state, &shadow);
	setWidgetState (widget, flags, state);

	switch (paint_what) {
		case BUTTON_PAINT:
		{
			QRect r (rect);
			detail = "button";
			if (kind == SCROLL_HBAR_WIDGET || kind == SCROLL_VBAR_WIDGET) {
				bool horizontal = flags & Style_Horizontal;
				detail = horizontal ? "hscrollbar" : "vscrollbar";

				/* Fool ClearLooks, so it knows these are corner step buttons. */
				int x, y, w, h;
				rect.rect (&x, &y, &w, &h);
				if (side == LeftSide)
					w = 100;
				else if (side == TopSide)
					h = 100;
				else if (side == RightSide) {
					x = 0;
					w = rect.x() + rect.width();
				}
				else if (side == BottomSide) {
					y = 0;
					h = rect.y() + rect.height();
				}
				widget->allocation.x = x;
				widget->allocation.y = y;
				widget->allocation.width = w;
				widget->allocation.height = h;
			}
			else if (kind == SPIN_WIDGET)
				detail = (side == TopSide) ? "spinbutton_up" : "spinbutton_down";
			else if (kind == BUTTON_WIDGET) {
				Dimension border;
				if (flags & Style_ButtonDefault) {
					gtk_paint_box (widget->style, drawable, state, GTK_SHADOW_OUT, area,
					               widget, "buttondefault", EXPAND_RECT (rect));	
					border = BUTTON_DEFAULT_BORDER_DIM;
				}
				else
					border = BUTTON_BORDER_DIM;
				r.setRect (r.x() + dimension (border, kind, LeftSide),
				           r.y() + dimension (border, kind, RightSide),
				           r.width() - dimension (border, kind, LeftSide)
				                     - dimension (border, kind, RightSide),
				           r.height() - dimension (border, kind, TopSide)
				                      - dimension (border, kind, BottomSide));
			}
			else
				detail = "button";

			if (kind == TOOL_ITEM_WIDGET) {
				widget = getWidget (GTK_TYPE_BUTTON);
				setWidgetState (widget, flags, state);
			}

			gtk_paint_box (widget->style, drawable, state, GTK_SHADOW_OUT, area,
			               widget, detail, EXPAND_RECT (r));	
			break;
		}
		case CHECK_INDICATOR_PAINT:
			if (kind == CHECK_WIDGET)
				detail = "checkbutton";
			else if (kind == LISTVIEW_WIDGET)
				detail = "cellcheck";
			else
				detail = "check";
			shadow = (flags & Style_On) ? GTK_SHADOW_IN : GTK_SHADOW_OUT;
			gtk_paint_check (widget->style, drawable, state, shadow, area,
			                 widget, detail, EXPAND_RECT (rect));
			break;
		case RADIO_INDICATOR_PAINT:
			if (kind == RADIO_WIDGET)
				detail = "radiobutton";
			else if (kind == LISTVIEW_WIDGET)
				detail = "cellradio";
			else
				detail = "option";
			shadow = (flags & Style_On) ? GTK_SHADOW_IN : GTK_SHADOW_OUT;
			gtk_paint_option (widget->style, drawable, state, shadow, area,
			                  widget, detail, EXPAND_RECT (rect));
			break;
		case FOCUS_PAINT:
			gtk_paint_focus (widget->style, drawable, GTK_STATE_NORMAL, area,
			                 widget, detail, EXPAND_RECT (rect));
			break;
		case FRAME_PAINT:
			if (kind == NOTEBOOK_WIDGET) {
				detail = "notebook";
				shadow = GTK_SHADOW_OUT;
			}
			else if (kind == PROGRESS_BAR_WIDGET) {
				detail = "trough";
				shadow = GTK_SHADOW_IN;
			}
			gtk_paint_box (widget->style, drawable, GTK_STATE_NORMAL, shadow,
			               area, widget, detail, EXPAND_RECT (rect));
			break;
		case BACKGROUND_PAINT:
			if (kind == MENU_BAR_WIDGET)
				detail = "menubar";
			else if (kind == MENU_WINDOW_WIDGET)
				detail = "menu";
			gtk_paint_box (widget->style, drawable, GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			               area, widget, detail, EXPAND_RECT (rect));
			break;
		case HIGHLIGHT_PAINT:
			if (state != GTK_STATE_INSENSITIVE) {
				if (kind == CHECK_WIDGET || kind == RADIO_WIDGET)
					gtk_paint_flat_box (widget->style, drawable, GTK_STATE_PRELIGHT,
					                    GTK_SHADOW_ETCHED_OUT, area, widget,
					                    "checkbutton", EXPAND_RECT (rect));
				else if (kind == MENU_ITEM_WIDGET) {
					gtk_widget_style_get (widget, "selected-shadow-type", &shadow, NULL);
					gtk_paint_box (widget->style, drawable, GTK_STATE_PRELIGHT, shadow,
					               area, widget, "menuitem", EXPAND_RECT (rect));
				}
			}
			break;
		case ENTRY_PAINT:
			if (state == GTK_STATE_PRELIGHT)
				state = GTK_STATE_NORMAL;
			if (kind == TEXT_VIEW_WIDGET)
				gtk_paint_shadow (widget->style, drawable, GTK_STATE_NORMAL,
				                  GTK_SHADOW_OUT, area, widget, "scrolled_window",
				                  EXPAND_RECT (rect));
			else {
				gtk_paint_flat_box (widget->style, drawable, state, GTK_SHADOW_NONE,
				                    area, widget, "entry_bg", EXPAND_RECT (rect));
				gtk_paint_shadow (widget->style, drawable, GTK_STATE_NORMAL, GTK_SHADOW_IN,
				                  area, widget, "entry", EXPAND_RECT (rect));
			}
			break;
		case ARROW_PAINT:
		{
			if (kind == SCROLL_HBAR_WIDGET)
				detail = "hscrollbar";
			else if (kind == SCROLL_VBAR_WIDGET)
				detail = "vscrollbar";
			else if (kind == SPIN_WIDGET)
				detail = (side == TopSide) ? "spinbutton_up" : "spinbutton_down";
			else if (kind == MENU_ITEM_WIDGET)
				detail = "menuitem";

			GtkArrowType arrow;
			if (side == LeftSide)
				arrow = GTK_ARROW_LEFT;
			else if (side == RightSide)
				arrow = GTK_ARROW_RIGHT;
			else if (side == TopSide)
				arrow = GTK_ARROW_UP;
			else
				arrow = GTK_ARROW_DOWN;

			gtk_paint_arrow (widget->style, drawable, state, shadow, area,
			                 widget, detail, arrow, TRUE, EXPAND_RECT (rect));
			break;
		}
		case SPIN_BUTTONS_BOX_PAINT:
		{
			GtkShadowType shadow;
			gtk_widget_style_get (widget, "shadow-type", &shadow, NULL);
			if (shadow != GTK_SHADOW_NONE)
				gtk_paint_box (widget->style, drawable, GTK_STATE_NORMAL, shadow,
				               area, widget, "spinbutton", EXPAND_RECT (rect));
			break;
		}
		case SCROLL_HANDLE_PAINT:
		{
			bool horizontal = flags & Style_Horizontal;
			GtkOrientation ori = horizontal ? GTK_ORIENTATION_HORIZONTAL
			                                : GTK_ORIENTATION_VERTICAL;
			if (kind == HSLIDER_WIDGET || kind == VSLIDER_WIDGET)
				detail = horizontal ? "hscale" : "vscale";
			else
				detail = "slider";
			gtk_paint_slider (widget->style, drawable, state, GTK_SHADOW_OUT, area,
			                  widget, detail, EXPAND_RECT (rect), ori);
			break;
		}
		case SCROLL_GROOVE_PAINT:
			gtk_paint_box (widget->style, drawable, state, GTK_SHADOW_IN, area,
			               widget, "trough", EXPAND_RECT (rect));
			break;
		case SIZE_GRIP_PAINT:
			gtk_paint_resize_grip (widget->style, drawable, GTK_STATE_NORMAL, NULL, widget,
			                       detail, GDK_WINDOW_EDGE_SOUTH_EAST, EXPAND_RECT (rect));
			break;
		case SEPARATOR_PAINT:
		{
			bool horizontal = flags & Style_Horizontal;
			detail = horizontal ? "hseparator" : "vseparator";
			if (kind == MENU_ITEM_WIDGET)
				detail = "menuitem";
			if (horizontal)
				gtk_paint_hline (widget->style, drawable, state, area, widget,
				                 detail, rect.x(), rect.x() + rect.width(),
				                 rect.y() + (rect.height() - widget->style->ythickness)/2);
			else
				gtk_paint_vline (widget->style, drawable, state, area, widget,
				                 detail, rect.y(), rect.y() + rect.height(),
				                 rect.x() + (rect.width() - widget->style->xthickness)/2);
			break;
		}
		case SPLITTER_PAINT:
		{
			GtkOrientation ori = flags & Style_Horizontal ? GTK_ORIENTATION_HORIZONTAL
			                                              : GTK_ORIENTATION_VERTICAL;
			gtk_paint_handle (widget->style, drawable, state, GTK_SHADOW_NONE,
			                  area, widget, "paned", EXPAND_RECT (rect), ori);
			break;
		}
		case HEADER_PAINT:
			if (kind == NOTEBOOK_WIDGET) {
				GtkPositionType pos;
				switch (side) {
					case LeftSide:
						pos = GTK_POS_RIGHT;
						break;
					case RightSide:
						pos = GTK_POS_LEFT;
						break;
					case BottomSide:
						pos = GTK_POS_TOP;
						break;
					case TopSide:
					default:
						pos = GTK_POS_BOTTOM;
						break;
				}
				if (flags & Style_Selected)
					state = GTK_STATE_NORMAL;
				else
					state = GTK_STATE_ACTIVE;
				gtk_paint_extension (widget->style, drawable, state, GTK_SHADOW_OUT,
				                     area, widget, "tab", EXPAND_RECT (rect), pos);
			}
			else if (kind == LISTVIEW_WIDGET) {
				widget = getWidget (GTK_TYPE_TREE_VIEW);
				widget = gtk_tree_view_get_column (GTK_TREE_VIEW (widget), 1)->button;
					setWidgetState (widget, flags, state);
				gtk_paint_box (widget->style, drawable, state, GTK_SHADOW_OUT, area,
				               widget, "button", EXPAND_RECT (rect));
			}
			break;
		case PROGRESS_BAR_PAINT:
			gtk_paint_box (widget->style, drawable, GTK_STATE_PRELIGHT, GTK_SHADOW_OUT,
			               area, widget, "bar", EXPAND_RECT (rect));
			break;
	}	
}

static int borderSide (GtkBorder *border, QSimpleStyle::Side side) /*helper*/
{
	switch (side) {
		case QSimpleStyle::NoSide:
			return 0;
		case QSimpleStyle::LeftSide:
			return border->left;
		case QSimpleStyle::RightSide:
			return border->right;
		case QSimpleStyle::TopSide:
			return border->top;
		case QSimpleStyle::BottomSide:
			return border->bottom;
	}
	return 0;
}

int QGtkStyle::dimension (Dimension dim, WidgetKind kind, Side side) const
{
	GtkWidget *widget = getWidget (kind);
	int value = 0;
	switch (dim) {
		case WIDGET_THICKNESS_DIM:
			if (side == LeftSide || side == RightSide)
				value = widget->style->xthickness;
			else
				value = widget->style->ythickness;
			break;
		case FOCUS_BORDER_DIM:
			gtk_widget_style_get (widget, "focus-padding", &value, NULL);
			break;
		case FOCUS_WIDTH_DIM:
			gtk_widget_style_get (widget, "focus-line-width", &value, NULL);
			break;
		case CHILD_BORDER_DIM:
		{
			if (kind == MENU_BAR_WIDGET)
				gtk_widget_style_get (widget, "internal-padding", &value, NULL);
			else if (kind == BUTTON_WIDGET || kind == RADIO_WIDGET ||
			         kind == CHECK_WIDGET) {
#if GTK_CHECK_VERSION(2,10,0)
			GtkBorder *border = 0;
			gtk_widget_style_get (widget, "inner-border", &border, NULL);
			if (border) {
				value = borderSide (border, side);
				g_free (border);
			}
#else
			value = 1;
#endif
			}
			break;
		}
		case FOCUS_DOWN_SHIFT_DIM:
		{
			if (kind == BUTTON_WIDGET) {
				bool shift_focus;
				gtk_widget_style_get (widget, "displace-focus", &shift_focus, NULL);
				if (shift_focus)
					value = dimension (CHILD_DOWN_SHIFT_DIM, kind, side);
				else
					value = 0;
			}
			break;
		}
		case CHILD_DOWN_SHIFT_DIM:
			if (kind == SCROLL_HBAR_WIDGET || kind == SCROLL_VBAR_WIDGET) {
				if (side == LeftSide)
					gtk_widget_style_get (widget, "arrow-displacement-x", &value, NULL);
				else if (side == TopSide)
					gtk_widget_style_get (widget, "arrow-displacement-y", &value, NULL);
			}
			else if (kind == BUTTON_WIDGET) {
				// NOTE: we only shift for GtkButton, not derivates, because GTK+ doesn't
				// honor this attribute in those (and it may be set)
				if (side == LeftSide)
					gtk_widget_style_get (widget, "child-displacement-x", &value, NULL);
				else if (side == TopSide)
					gtk_widget_style_get (widget, "child-displacement-y", &value, NULL);
			}
			break;

		case BUTTON_BORDER_DIM:
		{
			if (kind == BUTTON_WIDGET) {
				GtkBorder *border = 0;
				gtk_widget_style_get (widget, "default-outside-border", &border, NULL);
				if (border) {
					value = borderSide (border, side);
					g_free (border);
				}
			}
			break;
		}
		case BUTTON_DEFAULT_BORDER_DIM:
		{
			if (kind == BUTTON_WIDGET) {
				GtkBorder *border = 0;
				gtk_widget_style_get (widget, "default-border", &border, NULL);
				if (border) {
					value = borderSide (border, side);
					g_free (border);
				}
			}
			break;
		}

		case INDICATOR_BORDER_DIM:
			gtk_widget_style_get (widget, "indicator-spacing", &value, NULL);
			break;
		case INDICATOR_WIDTH_DIM:
		case INDICATOR_HEIGHT_DIM:
			gtk_widget_style_get (widget, "indicator-size", &value, NULL);
			break;
		case COMBO_ARROW_BORDER_DIM:
			value = 4;
			break;
		case COMBO_ARROW_WIDTH_DIM:
			value = 10;
			break;
		case SPIN_ARROW_WIDTH_DIM:
			value = pango_font_description_get_size (widget->style->font_desc);
			value = MAX (PANGO_PIXELS (value), 6);
			value = value - value % 2;  /* force even */
			break;
		case SLIDER_FRAME_WIDTH_DIM:
			gtk_widget_style_get (widget, "slider-width", &value, NULL);
			break;
		case SLIDER_HANDLE_WIDTH_DIM:
			if (kind == SCROLL_HBAR_WIDGET || kind == SCROLL_VBAR_WIDGET)
				gtk_widget_style_get (widget, "slider-width", &value, NULL);
			else //if (kind == HSLIDER_WIDGET || kind == VSLIDER_WIDGET)
				gtk_widget_style_get (widget, "slider-length", &value, NULL);
			break;
		case SLIDER_HANDLE_MIN_LENGTH_DIM:
			if (kind == SCROLL_HBAR_WIDGET || kind == SCROLL_VBAR_WIDGET)
				gtk_widget_style_get (widget, "min-slider-length", &value, NULL);
			break;
		case SLIDER_TICKS_SPACE_DIM:
			gtk_widget_style_get (widget, "value-spacing", &value, NULL);
			break;
		case SCROLL_STEP_BUTTON_BORDER_DIM:
			gtk_widget_style_get (widget, "stepper-spacing", &value, NULL);
			break;
		case SCROLL_STEP_BUTTON_WIDTH_DIM:
			gtk_widget_style_get (widget, "stepper-size", &value, NULL);
			break;
		case SCROLL_SLIDER_BORDER_DIM:
			gtk_widget_style_get (widget, "trough-border", &value, NULL);
			break;
		case SPLITTER_WIDTH_DIM:
			widget = getWidget (GTK_TYPE_VPANED);
			gtk_widget_style_get (widget, "handle-size", &value, NULL);
			break;

		case MENU_ITEM_SPACING_DIM:
			gtk_widget_style_get (widget, "horizontal-padding", &value, NULL);
			break;
		case MENU_ITEM_ARROW_SPACING_DIM:
			gtk_widget_style_get (widget, "arrow-spacing", &value, NULL);
			break;
		case MENU_ITEM_ICON_SPACING_DIM:
			gtk_widget_style_get (widget, "toggle-spacing", &value, NULL);
			break;
		case MENU_ITEM_SEPARATOR_HEIGHT_DIM:
			value = 4;  // hard coded from gtkmenuitem.c
			break;
		case MENU_ITEM_CHECK_SIZE_DIM:
			widget = getWidget (GTK_TYPE_CHECK_MENU_ITEM);
			gtk_widget_style_get (widget, "indicator-size", &value, NULL);
			break;
		case POPUP_MENU_VERTICAL_BORDER_DIM:
			gtk_widget_style_get (widget, "vertical-padding", &value, NULL);
			break;
		case NOTEBOOK_TAB_OVERLAP_DIM:
			value = 2;  // hard coded from gtknotebook.c
			break;
	}
	return value;
}

/* Hints about the feeling of some elements. */
int QGtkStyle::styleHint (StyleHint stylehint, const QWidget *qwidget,
                           const QStyleOption &opt, QStyleHintReturn *returnData) const

{
	// Due to the poor Qt description of the hints and lack of time to do
	// exhaustive testing to GTK+ feelings, some values may be wrong.
	// Please verify.
	switch (stylehint) {
		case SH_EtchDisabledText:
			// FIXME: verify what this does.
			return 1;
		case SH_UnderlineAccelerator:
			return 1;

		case SH_ScrollBar_MiddleClickAbsolutePosition:
		case SH_ScrollBar_ScrollWhenPointerLeavesControl:
			return 1;
		case SH_ScrollBar_LeftClickAbsolutePosition:
		case SH_ScrollBar_StopMouseOverSlider:
			return 0;
		case SH_ScrollView_FrameOnlyAroundContents:
			// It seems this may change in future versions of GTK+
			return 0;
		case SH_ItemView_ChangeHighlightOnFocus:
			// FIXME: verify what this does.
			return 1;

		case SH_Slider_SnapToValue:
		case SH_Slider_SloppyKeyEvents:
			return 1;

		case SH_GroupBox_TextLabelVerticalAlignment:
			return Qt::AlignVCenter;
		case SH_GroupBox_TextLabelColor:
			return 0x000000;
		case SH_ToolBox_SelectedPageTitleBold:
			// FIXME: verify what this does.
			return 5;

		case SH_PopupMenu_AllowActiveAndDisabled:
		case SH_PopupMenu_SpaceActivatesItem:
			return 0;
		case SH_PopupMenu_SubMenuPopupDelay:
		{
			GtkSettings *settings = gtk_settings_get_default();
			GValue delay = { 0 };
			g_value_init (&delay, G_TYPE_INT);
			g_object_get_property (G_OBJECT (settings), "gtk-menu-bar-popup-delay", &delay);
			return g_value_get_int (&delay);
		}
		case SH_PopupMenu_Scrollable:
			return 0;
		case SH_PopupMenu_SloppySubMenus:
			return 1;
		case SH_MenuBar_AltKeyNavigation:
			// This alt key navigation refers to click Alt, and then use
			// the numeric pad.
			return 0;
		case SH_ComboBox_ListMouseTracking:
		case SH_PopupMenu_MouseTracking:
			return 1;
		case SH_MenuBar_MouseTracking:
			return 0;
		case SH_ComboBox_Popup:
		{
			const QComboBox *qcombo = static_cast <const QComboBox *> (qwidget);
			if (!qcombo)
				return 1;
			if (qcombo->editable())
				return 0;
			GtkWidget *widget = getWidget (GTK_TYPE_COMBO_BOX);
			bool as_list;
			gtk_widget_style_get (widget, "appears-as-list", &as_list, NULL);
			return !as_list;
		}

		case SH_ProgressDialog_CenterCancelButton:
			return 0;
		case SH_ProgressDialog_TextLabelAlignment:
			return Qt::AlignLeft | Qt::AlignVCenter;
		case SH_PrintDialog_RightAlignButtons:
			return 1;
		case SH_MainWindow_SpaceBelowMenuBar:
			return 0;
		case SH_FontDialog_SelectAssociatedText:
			return 0;
		case SH_Widget_ShareActivation:
			// FIXME: verify what this does.
			return 1;
		case SH_DialogButtons_DefaultButton:
			// FIXME: verify is the return value is correct.
			return QMessageBox::Ok;

		case SH_Workspace_FillSpaceOnMaximize:
			return 1;
		case SH_TitleBar_NoBorder:
			return 0;

		case SH_BlinkCursorWhenTextSelected:
			return 0;
		case SH_RichText_FullWidthSelection:
			return 1;
		case SH_LineEdit_PasswordCharacter:
			return QChar (0x274d);

		case SH_Header_ArrowAlignment:
			return Qt::Right;
		case SH_ListViewExpand_SelectMouseType:
			// FIXME: verify is the return value is correct.
			return QEvent::MouseButtonRelease;
		case SH_Table_GridLineColor:
			// FIXME: verify the grid line color of GTK+ 2.10.0.
			return 0xbababa;

		case SH_TabBar_Alignment:
			return Qt::AlignLeft;
		case SH_TabBar_SelectMouseType:
			// FIXME: verify if the return value is correct.
			return QEvent::MouseButtonPress;
		case SH_TabBar_PreferNoArrows:
			return 0;

		default:
			return QCommonStyle::styleHint (stylehint, qwidget, opt, returnData);
	}
}

QPixmap QGtkStyle::stylePixmap (StylePixmap stylepixmap, const QWidget *widget,
                                 const QStyleOption &opt) const
{
	const gchar *stock_icon = 0;
	switch (stylepixmap) {
		case SP_MessageBoxInformation:
			stock_icon = GTK_STOCK_DIALOG_INFO;
			break;
		case SP_MessageBoxWarning:
			stock_icon = GTK_STOCK_DIALOG_WARNING;
			break;
		case SP_MessageBoxCritical:
			stock_icon = GTK_STOCK_DIALOG_ERROR;
			break;
		case SP_MessageBoxQuestion:
			stock_icon = GTK_STOCK_DIALOG_QUESTION;
			break;
		default:
			// we might want to also support SP_TitleBarCloseButton, but we don't
			// have icons for the other title bar buttons, so better leave the defaults
			break;
	}

// FIXME: we should better load the QPixmap directly from the file.
// we can get the icon theme with the GtkSetting.
// then, we do $GNOME_DIR/share/icons/$ICON_THEME
// not sure how to get Gnome's dir. maybe we should use $XDG_DATA_DIRS ?

	if (stock_icon) {
		GdkPixbuf *pixbuf;
		pixbuf = gtk_widget_render_icon (window_widget, stock_icon,
		                                 GTK_ICON_SIZE_DIALOG, NULL);
		if (pixbuf) {
			QPixmap pixmap = toQtPixmap (pixbuf, window_widget->style->bg_gc [0]);
			// TODO: add a alpha mask to the pixmap (pixmap.setMask (bitmap))
			g_object_unref (G_OBJECT (pixbuf));
			return pixmap;
		}
	}
	return QCommonStyle::stylePixmap (stylepixmap, widget, opt);
}

/**** Auxiliary functions ****/

GtkWidget *QGtkStyle::getWidget (GType type) const
{
	std::map <GType, GtkWidget *> &widgets =
		const_cast <QGtkStyle *> (this)->widgetsCache;

	GtkWidget *widget = widgets [type];
	if (!widget) {
		widget = gtk_widget_new (type, NULL);
		if (type == GTK_TYPE_WINDOW)
			const_cast <QGtkStyle *> (this)->window_widget = widget;
		else if (type == GTK_TYPE_FIXED) {
			gtk_container_add (GTK_CONTAINER (window_widget), widget);
			const_cast <QGtkStyle *> (this)->container_widget = widget;
		}
		else if (type == GTK_TYPE_MENU)
			;
		else
			gtk_container_add (GTK_CONTAINER (container_widget), widget);

		if (type == GTK_TYPE_TREE_VIEW) {
			// create a couple column, we'll need that
			gtk_tree_view_append_column (GTK_TREE_VIEW (widget),
			                             gtk_tree_view_column_new());
			gtk_tree_view_append_column (GTK_TREE_VIEW (widget),
			                             gtk_tree_view_column_new());
		}

		gtk_widget_realize (widget);
		gtk_widget_ensure_style (widget);
		widgets [type] = widget;
	}
	return widget;
}

GtkWidget *QGtkStyle::getWidget (WidgetKind kind) const
{
	switch (kind) {
		case BUTTON_WIDGET:
			return getWidget (GTK_TYPE_BUTTON);
		case RADIO_WIDGET:
			return getWidget (GTK_TYPE_RADIO_BUTTON);
		case CHECK_WIDGET:
			return getWidget (GTK_TYPE_CHECK_BUTTON);
		case COMBO_WIDGET:
			return getWidget (GTK_TYPE_COMBO_BOX);
		case COMBO_ENTRY_WIDGET:
			return getWidget (GTK_TYPE_COMBO_BOX_ENTRY);
		case LISTVIEW_WIDGET:
			return getWidget (GTK_TYPE_TREE_VIEW);
		case LINE_EDIT_WIDGET:
			return getWidget (GTK_TYPE_ENTRY);
		case TEXT_VIEW_WIDGET:
			return getWidget (GTK_TYPE_SCROLLED_WINDOW);
		case SCROLL_HBAR_WIDGET:
			return getWidget (GTK_TYPE_HSCROLLBAR);
		case SCROLL_VBAR_WIDGET:
			return getWidget (GTK_TYPE_VSCROLLBAR);
		case HSLIDER_WIDGET:
			return getWidget (GTK_TYPE_HSCALE);
		case VSLIDER_WIDGET:
			return getWidget (GTK_TYPE_VSCALE);
		case SPIN_WIDGET:
			return getWidget (GTK_TYPE_SPIN_BUTTON);
		case MENU_BAR_WIDGET:
			return getWidget (GTK_TYPE_MENU_BAR);
		case MENU_ITEM_WIDGET:
			return getWidget (GTK_TYPE_MENU_ITEM);
		case MENU_WINDOW_WIDGET:
			return getWidget (GTK_TYPE_MENU);
		case NOTEBOOK_WIDGET:
			return getWidget (GTK_TYPE_NOTEBOOK);
		case PROGRESS_BAR_WIDGET:
			return getWidget (GTK_TYPE_PROGRESS_BAR);
		case UNSPECIFIED_WIDGET:
		default:
			return window_widget;
	}
}

QColor QGtkStyle::toQtColor (const GdkColor &gcolor)
{
	// GdkColor representation is in 16-bit, not the 8-bit of Qt
	return QColor (gcolor.red >> 8, gcolor.green >> 8, gcolor.blue >> 8);
}

QPixmap QGtkStyle::toQtPixmap (GdkPixmap *gdk_pixmap, GdkGC *gdk_gc)
{
	int width, height;
	gdk_drawable_get_size (GDK_DRAWABLE (gdk_pixmap), &width, &height);

	QPixmap pix (width, height);
	GdkPixmap *gpix = gdk_pixmap_foreign_new (pix.handle());

	gdk_draw_rectangle (GDK_DRAWABLE (gpix), gdk_gc, TRUE, 0, 0, width, height);
	gdk_draw_drawable (GDK_DRAWABLE (gpix), gdk_gc, GDK_DRAWABLE (gdk_pixmap),
	                   0, 0, 0, 0, width, height);

	g_object_unref (G_OBJECT (gpix));
	return pix;
}

// GC to draw the background color/pixmap because GdkPixmap doesn't support
// transparency.
QPixmap QGtkStyle::toQtPixmap (GdkPixbuf *pixbuf, GdkGC *gc)
{
	int width, height;
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);

	QPixmap pix (width, height);
	GdkPixmap *gpix = gdk_pixmap_foreign_new (pix.handle());
	gdk_drawable_set_colormap (GDK_DRAWABLE (gpix), gdk_colormap_get_system());

	gdk_draw_rectangle (GDK_DRAWABLE (gpix), gc, TRUE, 0, 0, width, height);
	gdk_draw_pixbuf (GDK_DRAWABLE (gpix), NULL, pixbuf, 0, 0, 0, 0, width,
	                 height, GDK_RGB_DITHER_NONE, 0, 0);
	g_object_unref (G_OBJECT (gpix));
	return pix;
}

GdkRectangle QGtkStyle::toGdkRect (const QRect &r)
{
	GdkRectangle grect = { r.x(), r.y(), r.width(), r.height() };
	return grect;
}

void QGtkStyle::toGtkFlags (const SFlags flags, GtkStateType *state,
                              GtkShadowType *shadow)
{
	if (!(flags & Style_Enabled))
		*state = GTK_STATE_INSENSITIVE;
	else if (flags & (Style_Down | Style_Active))
		*state = GTK_STATE_ACTIVE;
	else if (flags & Style_MouseOver)
		*state = GTK_STATE_PRELIGHT;
	else
		*state = GTK_STATE_NORMAL;

	if (flags & (Style_On | Style_Down | Style_Sunken))
		*shadow = GTK_SHADOW_IN;
	else if (flags & (Style_Off | Style_Raised))
		*shadow = GTK_SHADOW_OUT;
	else
		*shadow = GTK_SHADOW_NONE;
}

void QGtkStyle::setWidgetState (GtkWidget *widget, const SFlags flags,
                                GtkStateType state)
{
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_SENSITIVE);

	if (flags & (Style_Default | Style_ButtonDefault))
		GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_DEFAULT);
	if (flags & Style_HasFocus)
		GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);
	if (flags & Style_Enabled)
		GTK_WIDGET_SET_FLAGS (widget, GTK_SENSITIVE);

	gtk_widget_set_state (widget, state);
}
