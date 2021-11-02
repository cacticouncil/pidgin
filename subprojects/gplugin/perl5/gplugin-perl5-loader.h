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

#ifndef GPLUGIN_PERL_LOADER_H
#define GPLUGIN_PERL_LOADER_H

#include <gplugin.h>
#include <gplugin-native.h>

G_BEGIN_DECLS

#define GPLUGIN_PERL_TYPE_LOADER (gplugin_perl_loader_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginPerlLoader,
	gplugin_perl_loader,
	GPLUGIN_PERL,
	LOADER,
	GPluginLoader)

void gplugin_perl_loader_register(GTypeModule *module);

GPluginLoader *gplugin_perl_loader_new(void);

G_END_DECLS

#endif /* GPLUGIN_PERL_PLUGIN_LOADER_H */
