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
#ifndef GPLUGIN_TCC_LOADER_H
#define GPLUGIN_TCC_LOADER_H

#include <gplugin.h>
#include <gplugin-native.h>

G_BEGIN_DECLS

#define GPLUGIN_TCC_TYPE_LOADER (gplugin_tcc_loader_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginTccLoader,
	gplugin_tcc_loader,
	GPLUGIN_TCC,
	LOADER,
	GPluginLoader)

void gplugin_tcc_loader_register(GPluginNativePlugin *plugin);

GPluginLoader *gplugin_tcc_loader_new(void);

G_END_DECLS

#endif /* GPLUGIN_TCC_LOADER_H */
