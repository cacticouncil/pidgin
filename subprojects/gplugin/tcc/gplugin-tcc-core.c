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

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include "gplugin-tcc-loader.h"
#include "gplugin-tcc-plugin.h"

static GPluginLoader *loader = NULL;

G_MODULE_EXPORT GPluginPluginInfo *
gplugin_query(G_GNUC_UNUSED GError **error)
{
	const gchar *const authors[] = {"Eion Robb <eion@robbmob.com>", NULL};

	/* clang-format off */
	return gplugin_plugin_info_new(
		"gplugin/tcc-loader",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"internal", TRUE,
		"load-on-query", TRUE,
		"name", "C source plugin loader",
		"version", GPLUGIN_VERSION,
		"license-id", "LGPL-2.0-or-later",
		"summary", "A plugin that can load C source plugins",
		"description", "This plugin allows the loading of plugins written in "
		               "C.",
		"authors", authors,
		"website", GPLUGIN_WEBSITE,
		"category", "loaders",
		"unloadable", FALSE,
		NULL);
	/* clang-format on */
}

G_MODULE_EXPORT gboolean
gplugin_load(GPluginNativePlugin *plugin, GError **error)
{
	GPluginManager *manager = NULL;

	gplugin_tcc_loader_register(plugin);
	gplugin_tcc_plugin_register(plugin);

	manager = gplugin_manager_get_default();

	loader = gplugin_tcc_loader_new();
	if(!gplugin_manager_register_loader(manager, loader, error)) {
		g_clear_object(&loader);

		return FALSE;
	}

	return TRUE;
}

G_MODULE_EXPORT gboolean
gplugin_unload(
	G_GNUC_UNUSED GPluginNativePlugin *plugin,
	gboolean shutdown,
	GError **error)
{
	GPluginManager *manager = NULL;
	gboolean ret = FALSE;

	if(!shutdown) {
		g_set_error_literal(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("The TCC loader can not be unloaded"));

		return FALSE;
	}

	manager = gplugin_manager_get_default();
	ret = gplugin_manager_unregister_loader(manager, loader, error);
	g_clear_object(&loader);

	return ret;
}
