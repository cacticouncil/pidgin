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

#ifndef GPLUGIN_GTK_VIEWER_WINDOW_H
#define GPLUGIN_GTK_VIEWER_WINDOW_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#define GPLUGIN_GTK_VIEWER_TYPE_WINDOW (gplugin_gtk_viewer_window_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginGtkViewerWindow,
	gplugin_gtk_viewer_window,
	GPLUGIN_GTK_VIEWER,
	WINDOW,
	GtkWindow)

G_BEGIN_DECLS

GtkWidget *gplugin_gtk_viewer_window_new(void);

G_END_DECLS

#endif /* GPLUGIN_GTK_VIEWER_WINDOW_H */
