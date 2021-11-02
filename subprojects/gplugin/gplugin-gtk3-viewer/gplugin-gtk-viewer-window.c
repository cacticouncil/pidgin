/*
 * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */
#include "gplugin-gtk-viewer-window.h"

#include <gtk/gtk.h>

struct _GPluginGtkViewerWindow {
	GtkWindow parent;
};

G_DEFINE_TYPE(
	GPluginGtkViewerWindow,
	gplugin_gtk_viewer_window,
	GTK_TYPE_WINDOW)

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
gplugin_gtk_viewer_window_init(GPluginGtkViewerWindow *window)
{
	gtk_widget_init_template(GTK_WIDGET(window));
}

static void
gplugin_gtk_viewer_window_class_init(GPluginGtkViewerWindowClass *klass)
{
	gtk_widget_class_set_template_from_resource(
		GTK_WIDGET_CLASS(klass),
		"/org/imfreedom/keep/gplugin/gplugin/viewer/window.ui");
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
gplugin_gtk_viewer_window_new(void)
{
	return GTK_WIDGET(g_object_new(GPLUGIN_GTK_VIEWER_TYPE_WINDOW, NULL));
}
