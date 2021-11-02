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

#ifndef GPLUGIN_PERL_PLUGIN_H
#define GPLUGIN_PERL_PLUGIN_H

#include <gplugin.h>
#include <gplugin-native.h>

#define PERL_NO_GET_CONTEXT
#ifdef __FreeBSD__
/* On FreeBSD, perl.h includes libutil.h (which doesn't appear necessary), and
 * that defines some generic 'properties' type. So use the preprocessor to hide
 * that name. */
#define properties freebsd_properties
#endif
#include <EXTERN.h>
#include <perl.h>
/* perl define's _() to something completely different that we don't use.  So
 * we undef it so that we can use it for gettext.
 */
#undef _
#ifdef __FreeBSD__
#undef properties
#endif

G_BEGIN_DECLS

#define GPLUGIN_PERL_TYPE_PLUGIN (gplugin_perl_plugin_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginPerlPlugin,
	gplugin_perl_plugin,
	GPLUGIN_PERL,
	PLUGIN,
	GObject)

void gplugin_perl_plugin_register(GTypeModule *module);

PerlInterpreter *gplugin_perl_plugin_get_interpreter(GPluginPerlPlugin *plugin);

G_END_DECLS

#endif /* GPLUGIN_PERL_PLUGIN_H */
