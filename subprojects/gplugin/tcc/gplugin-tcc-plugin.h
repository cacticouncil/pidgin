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
#ifndef GPLUGIN_TCC_PLUGIN_H
#define GPLUGIN_TCC_PLUGIN_H

#include <gplugin.h>
#include <gplugin-native.h>

#include <libtcc.h>

G_BEGIN_DECLS

#define GPLUGIN_TCC_TYPE_PLUGIN (gplugin_tcc_plugin_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginTccPlugin,
	gplugin_tcc_plugin,
	GPLUGIN_TCC,
	PLUGIN,
	GObject)

void gplugin_tcc_plugin_register(GPluginNativePlugin *native);

TCCState *gplugin_tcc_plugin_get_state(GPluginTccPlugin *plugin);

typedef GPluginPluginInfo *(*GPluginTccPluginQueryFunc)(GError **error);
typedef gboolean (
	*GPluginTccPluginLoadFunc)(GPluginNativePlugin *plugin, GError **error);
typedef gboolean (
	*GPluginTccPluginUnloadFunc)(GPluginNativePlugin *plugin, GError **error);

G_END_DECLS

#endif /* GPLUGIN_TCC_PLUGIN_H */
