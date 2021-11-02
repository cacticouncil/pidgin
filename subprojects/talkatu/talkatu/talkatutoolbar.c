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

#include "talkatu/talkatutoolbar.h"

/**
 * SECTION:talkatutoolbar
 * @Title: Toolbar
 * @Short_description: A simple formatting toolbar
 *
 * Talkatu provides a simple formatting toolbar that gives the user the ability
 * to format any #TalkatuBuffer via the formatting actions.
 */

/**
 * TALKATU_TYPE_TOOLBAR:
 *
 * The standard _get_type macro for #TalkatuToolbar.
 */

/**
 * TalkatuToolbar:
 *
 * The formatting toolbar for #TalkatuView's.  It's based on the
 * #GSimpleActionGroup's that #TalkatuBuffer's expose which makes it completely
 * autonomous.
 */
struct _TalkatuToolbar {
	GtkToolbar parent;
};

G_DEFINE_TYPE(TalkatuToolbar, talkatu_toolbar, GTK_TYPE_TOOLBAR)

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_toolbar_init(TalkatuToolbar *toolbar) {
	gtk_widget_init_template(GTK_WIDGET(toolbar));
}

static void
talkatu_toolbar_class_init(TalkatuToolbarClass *klass) {
	gtk_widget_class_set_template_from_resource(
		GTK_WIDGET_CLASS(klass),
		"/org/imfreedom/keep/talkatu/talkatu/ui/toolbar.ui"
	);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_toolbar_new:
 *
 * Creates a new #TalkatuToolbar that provides the user with an interface to
 * control the formatting of a #TalkatuBuffer.
 *
 * Returns: (transfer full): The new #TalkatuToolbar instance.
 */
GtkWidget *talkatu_toolbar_new(void) {
	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_TOOLBAR,
		NULL
	));
}
