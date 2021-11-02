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

#include "gplugin-tcc-loader.h"

#include <glib/gi18n-lib.h>

#include <libtcc.h>

#include "gplugin-tcc-plugin.h"

struct _GPluginTccLoader {
	GPluginLoader parent;
};

G_DEFINE_DYNAMIC_TYPE(
	GPluginTccLoader,
	gplugin_tcc_loader,
	GPLUGIN_TYPE_LOADER);

/******************************************************************************
 * GPluginLoaderInterface API
 *****************************************************************************/
static GSList *
gplugin_tcc_loader_supported_extensions(G_GNUC_UNUSED GPluginLoader *l)
{
	return g_slist_append(NULL, "c");
}

static GPluginPlugin *
gplugin_tcc_loader_query(
	GPluginLoader *loader,
	const gchar *filename,
	GError **error)
{
	GPluginPlugin *plugin = NULL;
	GPluginPluginInfo *info = NULL;
	TCCState *s = NULL;
	gpointer memneeded = NULL;
	int memsize = -1;

	GPluginTccPluginQueryFunc gplugin_query = NULL;

	s = tcc_new();

	tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

	if(tcc_add_file(s, filename) == -1) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			"couldn't load file %s",
			filename);

		tcc_delete(s);

		return NULL;
	}

	/* copy code into memory */
	if((memsize = tcc_relocate(s, NULL)) < 0) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			"couldn't work out how much memory is needed");

		tcc_delete(s);
		return NULL;
	}

	memneeded = g_malloc0(memsize);
	if(tcc_relocate(s, memneeded) < 0) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			"could not relocate plugin into memory");

		tcc_delete(s);
		g_free(memneeded);
		return NULL;
	}

	gplugin_query =
		(GPluginTccPluginQueryFunc)tcc_get_symbol(s, "gplugin_query");
	if(gplugin_query == NULL) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			"no gplugin_query function found");

		tcc_delete(s);
		g_free(memneeded);
		return NULL;
	}

	info = gplugin_query(error);
	if(info == NULL) {
		tcc_delete(s);
		g_free(memneeded);
		return NULL;
	}

	/* clang-format off */
	plugin = g_object_new(
		GPLUGIN_TCC_TYPE_PLUGIN,
		"filename", filename,
		"loader", loader,
		"state", s,
		"memory", memneeded,
		"info", info,
		NULL);
	/* clang-format on */

	return plugin;
}

static gboolean
gplugin_tcc_loader_load(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginTccPluginLoadFunc gplugin_load = NULL;
	TCCState *s = gplugin_tcc_plugin_get_state(GPLUGIN_TCC_PLUGIN(plugin));

	gplugin_load = (GPluginTccPluginLoadFunc)tcc_get_symbol(s, "gplugin_load");
	if(gplugin_load == NULL) {
		g_set_error(error, GPLUGIN_DOMAIN, 0, "no gplugin_load function found");

		return FALSE;
	}

	return gplugin_load(GPLUGIN_NATIVE_PLUGIN(plugin), error);
}

static gboolean
gplugin_tcc_loader_unload(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginTccPluginLoadFunc gplugin_unload = NULL;
	TCCState *s = gplugin_tcc_plugin_get_state(GPLUGIN_TCC_PLUGIN(plugin));

	gplugin_unload =
		(GPluginTccPluginUnloadFunc)tcc_get_symbol(s, "gplugin_unload");
	if(gplugin_unload == NULL) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			"no gplugin_unload function found");

		return FALSE;
	}

	return gplugin_unload(GPLUGIN_NATIVE_PLUGIN(plugin), error);
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
gplugin_tcc_loader_init(G_GNUC_UNUSED GPluginTccLoader *loader)
{
}

static void
gplugin_tcc_loader_class_finalize(G_GNUC_UNUSED GPluginTccLoaderClass *klass)
{
}

static void
gplugin_tcc_loader_class_init(GPluginTccLoaderClass *klass)
{
	GPluginLoaderClass *loader_class = GPLUGIN_LOADER_CLASS(klass);

	loader_class->supported_extensions =
		gplugin_tcc_loader_supported_extensions;
	loader_class->query = gplugin_tcc_loader_query;
	loader_class->load = gplugin_tcc_loader_load;
	loader_class->unload = gplugin_tcc_loader_unload;
}

/******************************************************************************
 * API
 *****************************************************************************/
void
gplugin_tcc_loader_register(GPluginNativePlugin *plugin)
{
	gplugin_tcc_loader_register_type(G_TYPE_MODULE(plugin));
}

GPluginLoader *
gplugin_tcc_loader_new(void)
{
	/* clang-format off */
	return GPLUGIN_LOADER(g_object_new(
		GPLUGIN_TCC_TYPE_LOADER,
		"id", "gplugin-tcc",
		NULL));
	/* clang-format on */
}
