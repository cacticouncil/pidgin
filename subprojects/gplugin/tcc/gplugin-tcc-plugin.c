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

#include "gplugin-tcc-plugin.h"

#include <libtcc.h>

/******************************************************************************
 * Typedefs
 *****************************************************************************/
struct _GPluginTccPlugin {
	GObject parent;

	TCCState *s;
	gpointer mem;

	gchar *filename;
	GPluginLoader *loader;
	GPluginPluginInfo *info;
	GPluginPluginState state;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_TCC_STATE,
	PROP_MEM,
	N_PROPERTIES,

	/* overrides */
	PROP_FILENAME = N_PROPERTIES,
	PROP_LOADER,
	PROP_INFO,
	PROP_STATE
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

/* I hate forward declarations... */
static void gplugin_tcc_plugin_iface_init(GPluginPluginInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(
	GPluginTccPlugin,
	gplugin_tcc_plugin,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE(GPLUGIN_TYPE_PLUGIN, gplugin_tcc_plugin_iface_init));

/******************************************************************************
 * GPluginPlugin Implementation
 *****************************************************************************/
static void
gplugin_tcc_plugin_iface_init(G_GNUC_UNUSED GPluginPluginInterface *iface)
{
}

/******************************************************************************
 * Object Stuff
 *****************************************************************************/
static void
gplugin_tcc_plugin_get_property(
	GObject *obj,
	guint param_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginTccPlugin *plugin = GPLUGIN_TCC_PLUGIN(obj);

	switch(param_id) {
		case PROP_TCC_STATE:
			g_value_set_pointer(value, gplugin_tcc_plugin_get_state(plugin));
			break;

		/* overrides */
		case PROP_FILENAME:
			g_value_set_string(value, plugin->filename);
			break;
		case PROP_LOADER:
			g_value_set_object(value, plugin->loader);
			break;
		case PROP_INFO:
			g_value_set_object(value, plugin->info);
			break;
		case PROP_STATE:
			g_value_set_enum(value, plugin->state);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_tcc_plugin_set_property(
	GObject *obj,
	guint param_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginTccPlugin *plugin = GPLUGIN_TCC_PLUGIN(obj);

	switch(param_id) {
		case PROP_TCC_STATE:
			plugin->s = g_value_get_pointer(value);
			break;
		case PROP_MEM:
			plugin->mem = g_value_get_pointer(value);
			break;

		/* overrides */
		case PROP_FILENAME:
			plugin->filename = g_value_dup_string(value);
			break;
		case PROP_LOADER:
			plugin->loader = g_value_dup_object(value);
			break;
		case PROP_INFO:
			plugin->info = g_value_dup_object(value);
			break;
		case PROP_STATE:
			plugin->state = g_value_get_enum(value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_tcc_plugin_finalize(GObject *obj)
{
	GPluginTccPlugin *plugin = GPLUGIN_TCC_PLUGIN(obj);

	g_clear_pointer(&plugin->s, tcc_delete);
	g_clear_pointer(&plugin->mem, g_free);

	g_clear_pointer(&plugin->filename, g_free);
	g_clear_object(&plugin->loader);
	g_clear_object(&plugin->info);

	G_OBJECT_CLASS(gplugin_tcc_plugin_parent_class)->finalize(obj);
}

static void
gplugin_tcc_plugin_init(G_GNUC_UNUSED GPluginTccPlugin *plugin)
{
}

static void
gplugin_tcc_plugin_class_finalize(G_GNUC_UNUSED GPluginTccPluginClass *klass)
{
}

static void
gplugin_tcc_plugin_class_init(GPluginTccPluginClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_tcc_plugin_get_property;
	obj_class->set_property = gplugin_tcc_plugin_set_property;
	obj_class->finalize = gplugin_tcc_plugin_finalize;

	properties[PROP_TCC_STATE] = g_param_spec_pointer(
		"tcc-state",
		"tcc-state",
		"The TCC compilation context for the plugin",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_MEM] = g_param_spec_pointer(
		"memory",
		"memory",
		"The memory allocated for the symbol table for the plugin",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* add our overrides */
	g_object_class_override_property(obj_class, PROP_FILENAME, "filename");
	g_object_class_override_property(obj_class, PROP_LOADER, "loader");
	g_object_class_override_property(obj_class, PROP_INFO, "info");
	g_object_class_override_property(obj_class, PROP_STATE, "state");
}

/******************************************************************************
 * API
 *****************************************************************************/
void
gplugin_tcc_plugin_register(GPluginNativePlugin *native)
{
	gplugin_tcc_plugin_register_type(G_TYPE_MODULE(native));
}

TCCState *
gplugin_tcc_plugin_get_state(GPluginTccPlugin *plugin)
{
	g_return_val_if_fail(GPLUGIN_TCC_IS_PLUGIN(plugin), NULL);

	return plugin->s;
}
