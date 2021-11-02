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

#if !defined(GPLUGIN_GTK_GLOBAL_HEADER_INSIDE) && \
	!defined(GPLUGIN_GTK_COMPILATION)
#error "only <gplugin-gtk.h> may be included directly"
#endif

#ifndef GPLUGIN_GTK_STORE_H
#define GPLUGIN_GTK_STORE_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPLUGIN_GTK_TYPE_STORE (gplugin_gtk_store_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginGtkStore,
	gplugin_gtk_store,
	GPLUGIN_GTK,
	STORE,
	GtkListStore)

typedef enum {
	GPLUGIN_GTK_STORE_ENABLED_COLUMN,
	GPLUGIN_GTK_STORE_LOADED_COLUMN,
	GPLUGIN_GTK_STORE_PLUGIN_COLUMN,
	GPLUGIN_GTK_STORE_MARKUP_COLUMN,

	/*< private >*/
	GPLUGIN_GTK_STORE_N_COLUMNS,
} GPluginGtkStoreColumns;

GPluginGtkStore *gplugin_gtk_store_new(void);

const GType *gplugin_gtk_store_get_column_types(void);

G_END_DECLS

#endif /* GPLUGIN_GTK_STORE_H */
