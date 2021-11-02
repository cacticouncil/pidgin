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

#include <gplugin.h>

#include <gplugin-gtk-store.h>
#include <gplugin-gtk-view.h>

/**
 * GPluginGtkView:
 *
 * A [class@Gtk.TreeView] widget that displays all the plugins and some basic
 * information about them.
 */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginGtkView {
	GtkTreeView parent;

	gboolean show_internal;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_SHOW_INTERNAL,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
gplugin_gtk_view_plugin_toggled_cb(
	G_GNUC_UNUSED GtkCellRendererToggle *rend,
	gchar *path,
	gpointer data)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(data);
	GPluginManager *manager = NULL;
	GPluginPlugin *plugin = NULL;
	GPluginPluginState state;
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;
	GtkTreePath *tree_path = NULL;

	manager = gplugin_manager_get_default();

	tree_path = gtk_tree_path_new_from_string(path);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
	gtk_tree_model_get_iter(model, &iter, tree_path);
	gtk_tree_path_free(tree_path);

	gtk_tree_model_get(
		model,
		&iter,
		GPLUGIN_GTK_STORE_PLUGIN_COLUMN,
		&plugin,
		-1);

	if(!GPLUGIN_IS_PLUGIN(plugin)) {
		return;
	}

	state = gplugin_plugin_get_state(plugin);
	if(state == GPLUGIN_PLUGIN_STATE_LOADED) {
		GError *error = NULL;

		gplugin_manager_unload_plugin(manager, plugin, &error);

		if(error != NULL) {
			g_warning("Failed to unload plugin: %s", error->message);

			g_error_free(error);
		}
	} else {
		GError *error = NULL;

		gplugin_manager_load_plugin(manager, plugin, &error);

		if(error != NULL) {
			g_warning("Failed to load plugin: %s", error->message);

			g_error_free(error);
		}
	}

	g_object_unref(G_OBJECT(plugin));
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(GPluginGtkView, gplugin_gtk_view, GTK_TYPE_TREE_VIEW)

static void
gplugin_gtk_view_set_property(
	GObject *obj,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(obj);

	switch(prop_id) {
		case PROP_SHOW_INTERNAL:
			gplugin_gtk_view_set_show_internal(
				view,
				g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_view_get_property(
	GObject *obj,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkView *view = GPLUGIN_GTK_VIEW(obj);

	switch(prop_id) {
		case PROP_SHOW_INTERNAL:
			g_value_set_boolean(
				value,
				gplugin_gtk_view_get_show_internal(view));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_view_init(GPluginGtkView *view)
{
	GtkTreeViewColumn *col = NULL;
	GtkCellRenderer *rend = NULL;

	/* create the first column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Enabled");
	gtk_tree_view_column_set_resizable(col, FALSE);

	rend = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(col, rend, FALSE);
	g_signal_connect(
		G_OBJECT(rend),
		"toggled",
		G_CALLBACK(gplugin_gtk_view_plugin_toggled_cb),
		view);

	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	gtk_tree_view_column_set_attributes(
		col,
		rend,
		"active",
		GPLUGIN_GTK_STORE_LOADED_COLUMN,
		"sensitive",
		GPLUGIN_GTK_STORE_ENABLED_COLUMN,
		NULL);

	/* create the markup column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Name");
	gtk_tree_view_column_set_resizable(col, FALSE);

	rend = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, rend, TRUE);

	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	gtk_tree_view_column_set_attributes(
		col,
		rend,
		"markup",
		GPLUGIN_GTK_STORE_MARKUP_COLUMN,
		"sensitive",
		GPLUGIN_GTK_STORE_ENABLED_COLUMN,
		NULL);
}

static void
gplugin_gtk_view_class_init(GPluginGtkViewClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_gtk_view_get_property;
	obj_class->set_property = gplugin_gtk_view_set_property;

	/* properties */

	/**
	 * GPluginGtkView:show-internal:
	 *
	 * Whether or not to show internal plugins.
	 */
	properties[PROP_SHOW_INTERNAL] = g_param_spec_boolean(
		"show-internal",
		"show-internal",
		"Whether or not to show internal plugins",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_gtk_view_new:
 *
 * Creates a new [class@GPluginGtk3.View].
 *
 * Returns: (transfer full): The new view.
 */
GtkWidget *
gplugin_gtk_view_new(void)
{
	GObject *ret = NULL;
	GPluginGtkStore *store = gplugin_gtk_store_new();

	/* clang-format off */
	ret = g_object_new(
		GPLUGIN_GTK_TYPE_VIEW,
		"model", GTK_TREE_MODEL(store),
		NULL);
	/* clang-format on */

	return GTK_WIDGET(ret);
}

/**
 * gplugin_gtk_view_set_show_internal:
 * @view: The GTK view instance.
 * @show_internal: Whether or not to show internal plugins.
 *
 * This function will toggle whether or not the widget will show internal
 * plugins.
 */
void
gplugin_gtk_view_set_show_internal(GPluginGtkView *view, gboolean show_internal)
{
	g_return_if_fail(GPLUGIN_GTK_IS_VIEW(view));

	view->show_internal = show_internal;

	g_object_notify(G_OBJECT(view), "show-internal");
}

/**
 * gplugin_gtk_view_get_show_internal:
 * @view: The GTK view instance.
 *
 * Returns whether or not @view is showing internal plugins.
 */
gboolean
gplugin_gtk_view_get_show_internal(GPluginGtkView *view)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_VIEW(view), FALSE);

	return view->show_internal;
}
