/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include <talkatu/talkatuscrolledwindow.h>

/**
 * SECTION:talkatuscrolledwindow
 * @Title: Enhanced Scrolled Window
 * @Short_description: An enhanced GtkScrolledWindow
 *
 * This widget is a simple subclass of #GtkScrolledWindow that has helpers for
 * keyboard navigation as well as the ability to automatically scroll to the end
 * when new items are added if the widget was already scrolled all the way to
 * the bottom.
 */

/**
 * TALKATU_TYPE_SCROLLED_WINDOW:
 *
 * The standard _get_type macro for #TalkatuScrolledWindow.
 */

/**
 * TalkatuScrolledWindow:
 *
 * An enhanced #GtkScrolledWindow subclass.
 */
struct _TalkatuScrolledWindow {
	GtkScrolledWindow parent;

	GtkAdjustment *vadjustment;
	gboolean auto_scroll;
};

G_DEFINE_TYPE(TalkatuScrolledWindow, talkatu_scrolled_window,
              GTK_TYPE_SCROLLED_WINDOW)

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
talkatu_scrolled_window_vadjustment_changed_cb(GtkAdjustment *adjustment,
                                               gpointer data)
{
	TalkatuScrolledWindow *sw = TALKATU_SCROLLED_WINDOW(data);

	if(sw->auto_scroll) {
		gdouble upper, pagesize;

		upper = gtk_adjustment_get_upper(adjustment);
		pagesize = gtk_adjustment_get_page_size(adjustment);

		gtk_adjustment_set_value(adjustment, upper - pagesize);
	}
}

static void
talkatu_scrolled_window_vadjustment_value_changed_cb(GtkAdjustment *adjustment,
                                                     gpointer data)
{
	TalkatuScrolledWindow *sw = TALKATU_SCROLLED_WINDOW(data);
	gdouble current, upper, pagesize;

	current = gtk_adjustment_get_value(adjustment);
	upper = gtk_adjustment_get_upper(adjustment);
	pagesize = gtk_adjustment_get_page_size(adjustment);

	sw->auto_scroll = (current + pagesize >= upper);
}

static void
talkatu_scrolled_window_vadjustment_notify_cb(GObject *obj, GParamSpec *pspec,
                                              gpointer data)
{
	TalkatuScrolledWindow *sw = TALKATU_SCROLLED_WINDOW(obj);
	GtkAdjustment *adjustment = NULL;

	adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(obj));
	if(g_set_object(&sw->vadjustment, adjustment)) {
		sw->auto_scroll = TRUE;

		g_signal_connect(G_OBJECT(adjustment), "value-changed",
		                 G_CALLBACK(talkatu_scrolled_window_vadjustment_value_changed_cb),
		                 sw);
		g_signal_connect(G_OBJECT(adjustment), "changed",
		                 G_CALLBACK(talkatu_scrolled_window_vadjustment_changed_cb),
		                 sw);
	}
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_scrolled_window_init(TalkatuScrolledWindow *sw) {
	sw->auto_scroll = TRUE;

	g_signal_connect(G_OBJECT(sw), "notify::vadjustment",
	                 G_CALLBACK(talkatu_scrolled_window_vadjustment_notify_cb),
	                 NULL);
}

static void
talkatu_scrolled_window_finalize(GObject *obj) {
	TalkatuScrolledWindow *sw = TALKATU_SCROLLED_WINDOW(obj);

	g_clear_object(&sw->vadjustment);

	G_OBJECT_CLASS(talkatu_scrolled_window_parent_class)->finalize(obj);
}

static void
talkatu_scrolled_window_class_init(TalkatuScrolledWindowClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = talkatu_scrolled_window_finalize;
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_scrolled_window_new:
 * @hadjustment: The GtkAdjustment for the horizontal position.
 * @vadjustment: The GtkAdjustment for the vertical position.
 *
 * Creates a new #TalkatuScrolledWindow.
 *
 * Returns: (transfer full): The new #TalkatuScrolledWindow instance.
 */
GtkWidget *talkatu_scrolled_window_new(GtkAdjustment *hadjustment,
                                       GtkAdjustment *vadjustment)
{
	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_SCROLLED_WINDOW,
		"hadjustment", hadjustment,
		"vadjustment", vadjustment,
		NULL
	));
}

/**
 * talkatu_scrolled_window_page_up:
 * @sw: The #TalkatuScrolledWindow instance.
 *
 * Scrolls @sw up one page.
 */
void
talkatu_scrolled_window_page_up(TalkatuScrolledWindow *sw) {
	g_return_if_fail(TALKATU_IS_SCROLLED_WINDOW(sw));

	if(sw->vadjustment != NULL) {
		double value = 0.0;

		value = gtk_adjustment_get_value(sw->vadjustment);
		value -= gtk_adjustment_get_page_increment(sw->vadjustment);
		gtk_adjustment_set_value(sw->vadjustment, value);
	}
}

/**
 * talkatu_scrolled_window_page_down:
 * @sw: The #TalkatuScrolledWindow instance.
 *
 * Scrolls @sw down one page.
 */
void
talkatu_scrolled_window_page_down(TalkatuScrolledWindow *sw) {
	g_return_if_fail(TALKATU_IS_SCROLLED_WINDOW(sw));

	if(sw->vadjustment != NULL) {
		double value = 0.0;

		value = gtk_adjustment_get_value(sw->vadjustment);
		value += gtk_adjustment_get_page_increment(sw->vadjustment);
		gtk_adjustment_set_value(sw->vadjustment, value);
	}
}
