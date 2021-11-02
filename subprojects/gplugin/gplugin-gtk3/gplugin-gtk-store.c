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

/**
 * GPluginGtkStoreColumns:
 * @GPLUGIN_GTK_STORE_ENABLED_COLUMN: The disabled column.  This is used when a
 *                                    plugin is in a state that can't be
 *                                    changed.  So the row should be disabled.
 * @GPLUGIN_GTK_STORE_LOADED_COLUMN: The loaded column.
 * @GPLUGIN_GTK_STORE_PLUGIN_COLUMN: The plugin column.
 * @GPLUGIN_GTK_STORE_MARKUP_COLUMN: The markup column.
 *
 * An enum declaring the columns in a [class@GPluginGtk3.Store].
 */

/**
 * GPluginGtkStore:
 *
 * A [class@Gtk.ListStore] that contains all of the known plugins in GPlugin.
 */

struct _GPluginGtkStore {
	GtkListStore parent;
};

G_DEFINE_TYPE(GPluginGtkStore, gplugin_gtk_store, GTK_TYPE_LIST_STORE)

/******************************************************************************
 * Globals
 *****************************************************************************/
static const GType column_types[] = {
	G_TYPE_BOOLEAN,
	G_TYPE_BOOLEAN,
	G_TYPE_OBJECT,
	G_TYPE_STRING,
};

G_STATIC_ASSERT(G_N_ELEMENTS(column_types) == GPLUGIN_GTK_STORE_N_COLUMNS);

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
gplugin_gtk_store_add_plugin(GPluginGtkStore *store, GPluginPlugin *plugin)
{
	GtkTreeIter iter;
	GPluginPluginInfo *info = gplugin_plugin_get_info(plugin);
	GPluginPluginState state = gplugin_plugin_get_state(plugin);
	GString *str = g_string_new("");
	gchar *name = NULL, *summary = NULL;
	gboolean loaded = FALSE, enabled = TRUE;

	/* clang-format off */
	g_object_get(
		G_OBJECT(info),
		"name", &name,
		"summary", &summary,
		NULL);
	/* clang-format on */

	g_string_append_printf(
		str,
		"<b>%s</b>\n",
		(name) ? name : "<i>Unnamed</i>");
	g_string_append_printf(
		str,
		"%s",
		(summary) ? summary : "<i>No Summary</i>");

	g_free(name);
	g_free(summary);

	loaded = (state == GPLUGIN_PLUGIN_STATE_LOADED);
	if(state == GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED) {
		loaded = TRUE;
		enabled = FALSE;
	}

	gtk_list_store_append(GTK_LIST_STORE(store), &iter);
	gtk_list_store_set(
		GTK_LIST_STORE(store),
		&iter,
		GPLUGIN_GTK_STORE_LOADED_COLUMN,
		loaded,
		GPLUGIN_GTK_STORE_ENABLED_COLUMN,
		enabled,
		GPLUGIN_GTK_STORE_PLUGIN_COLUMN,
		g_object_ref(plugin),
		GPLUGIN_GTK_STORE_MARKUP_COLUMN,
		str->str,
		-1);

	g_string_free(str, TRUE);
	g_object_unref(G_OBJECT(info));
}

static void
gplugin_gtk_store_add_plugin_by_id(GPluginGtkStore *store, const gchar *id)
{
	GPluginManager *manager = NULL;
	GSList *plugins = NULL, *l = NULL;

	manager = gplugin_manager_get_default();

	plugins = gplugin_manager_find_plugins(manager, id);
	for(l = plugins; l; l = l->next)
		gplugin_gtk_store_add_plugin(store, GPLUGIN_PLUGIN(l->data));
	g_slist_free_full(plugins, g_object_unref);
}

static gboolean
gplugin_gtk_store_update_plugin_state_cb(
	GtkTreeModel *model,
	G_GNUC_UNUSED GtkTreePath *path,
	GtkTreeIter *iter,
	gpointer data)
{
	GPluginPlugin *plugin_a = GPLUGIN_PLUGIN(data);
	GPluginPlugin *plugin_b = NULL;
	gboolean ret = FALSE;

	gtk_tree_model_get(
		model,
		iter,
		GPLUGIN_GTK_STORE_PLUGIN_COLUMN,
		&plugin_b,
		-1);

	if(plugin_a == plugin_b) {
		GPluginPluginState state = gplugin_plugin_get_state(plugin_a);
		gboolean loaded = (state == GPLUGIN_PLUGIN_STATE_LOADED);
		gboolean enabled = TRUE;

		if(state == GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED) {
			loaded = TRUE;
			enabled = FALSE;
		}

		gtk_list_store_set(
			GTK_LIST_STORE(model),
			iter,
			GPLUGIN_GTK_STORE_LOADED_COLUMN,
			loaded,
			GPLUGIN_GTK_STORE_ENABLED_COLUMN,
			enabled,
			-1);

		/* tell gplugin_gtk_store_update_plugin_state that we're done */
		ret = TRUE;
	}

	g_object_unref(G_OBJECT(plugin_b));

	return ret;
}

static void
gplugin_gtk_store_update_plugin_state(
	GPluginGtkStore *store,
	GPluginPlugin *plugin)
{
	gtk_tree_model_foreach(
		GTK_TREE_MODEL(store),
		gplugin_gtk_store_update_plugin_state_cb,
		plugin);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
gplugin_gtk_store_plugin_loaded_cb(
	G_GNUC_UNUSED GObject *manager,
	GPluginPlugin *plugin,
	gpointer data)
{
	gplugin_gtk_store_update_plugin_state(GPLUGIN_GTK_STORE(data), plugin);
}

static void
gplugin_gtk_store_plugin_unloaded_cb(
	G_GNUC_UNUSED GObject *manager,
	GPluginPlugin *plugin,
	gpointer data)
{
	gplugin_gtk_store_update_plugin_state(GPLUGIN_GTK_STORE(data), plugin);
}

static void
gplugin_gtk_store_plugin_unload_failed_cb(
	G_GNUC_UNUSED GObject *manager,
	GPluginPlugin *plugin,
	gpointer data)
{
	gplugin_gtk_store_update_plugin_state(GPLUGIN_GTK_STORE(data), plugin);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
gplugin_gtk_store_constructed(GObject *obj)
{
	GPluginManager *manager = NULL;
	GList *l, *ids = NULL;

	G_OBJECT_CLASS(gplugin_gtk_store_parent_class)->constructed(obj);

	manager = gplugin_manager_get_default();

	ids = gplugin_manager_list_plugins(manager);
	for(l = ids; l; l = l->next)
		gplugin_gtk_store_add_plugin_by_id(
			GPLUGIN_GTK_STORE(obj),
			(const gchar *)l->data);
	g_list_free(ids);

	g_signal_connect_object(
		manager,
		"loaded-plugin",
		G_CALLBACK(gplugin_gtk_store_plugin_loaded_cb),
		obj,
		0);
	g_signal_connect_object(
		manager,
		"unloaded-plugin",
		G_CALLBACK(gplugin_gtk_store_plugin_unloaded_cb),
		obj,
		0);
	g_signal_connect_object(
		manager,
		"unload-plugin-failed",
		G_CALLBACK(gplugin_gtk_store_plugin_unload_failed_cb),
		obj,
		0);
}

static void
gplugin_gtk_store_init(GPluginGtkStore *store)
{
	GType *types = (GType *)gplugin_gtk_store_get_column_types();

	gtk_list_store_set_column_types(
		GTK_LIST_STORE(store),
		GPLUGIN_GTK_STORE_N_COLUMNS,
		types);
}

static void
gplugin_gtk_store_class_init(GPluginGtkStoreClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->constructed = gplugin_gtk_store_constructed;
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_gtk_store_new:
 *
 * Create a new #GPluginGtkStore which is a prepopulated [class@Gtk.TreeStore].
 *
 * Returns: (transfer full): A new list store prepopulated with all of the
 *          plugins.
 */
GPluginGtkStore *
gplugin_gtk_store_new(void)
{
	return GPLUGIN_GTK_STORE(g_object_new(GPLUGIN_GTK_TYPE_STORE, NULL));
}

/**
 * gplugin_gtk_store_get_column_types:
 *
 * Returns the columns that #GPluginGtkStore's will use.
 *
 * Returns: (transfer none): A list of [type@GLib.Type]'s for the columns that
 *          the store will use.
 */
const GType *
gplugin_gtk_store_get_column_types(void)
{
	return column_types;
}
