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
#include <gplugin-native.h>

#include "gplugin-perl5-loader.h"
#include "gplugin-perl5-plugin.h"

#undef _
#include <glib/gi18n-lib.h>

static GPluginLoader *loader = NULL;

static GPluginPluginInfo *
gplugin_perl5_query(G_GNUC_UNUSED GError **error)
{
	const gchar *const authors[] = {
		"Gary Kramlich <grim@reaperworld.com>",
		NULL,
	};

	/* clang-format off */
	return gplugin_plugin_info_new(
		"gplugin/perl5-loader",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"internal", TRUE,
		"load-on-query", TRUE,
		"name", "Perl5 plugin loader",
		"version", GPLUGIN_VERSION,
		"license-id", "LGPL-2.0-or-later",
		"summary", "A plugin that can load perl plugins",
		"description", "This plugin allows the loading of plugins written in "
		               "the perl programming language.",
		"authors", authors,
		"website", GPLUGIN_WEBSITE,
		"category", "loaders",
		"bind-global", TRUE,
		"unloadable", FALSE,
		NULL);
	/* clang-format on */
}

static gboolean
gplugin_perl5_load(GPluginPlugin *plugin, GError **error)
{
	GPluginManager *manager = NULL;

	manager = gplugin_manager_get_default();

	gplugin_perl_plugin_register(G_TYPE_MODULE(plugin));
	gplugin_perl_loader_register(G_TYPE_MODULE(plugin));

	loader = gplugin_perl_loader_new();
	if(!gplugin_manager_register_loader(manager, loader, error)) {
		g_clear_object(&loader);

		return FALSE;
	}

	return TRUE;
}

static gboolean
gplugin_perl5_unload(
	G_GNUC_UNUSED GPluginPlugin *plugin,
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
			_("The Perl5 loader can not be unloaded"));

		return FALSE;
	}

	manager = gplugin_manager_get_default();
	ret = gplugin_manager_unregister_loader(manager, loader, error);
	g_clear_object(&loader);

	return ret;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(gplugin_perl5)
