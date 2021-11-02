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

#include "gplugin-perl5-loader.h"

#include "gplugin-perl5-plugin.h"

#include <gperl.h>

struct _GPluginPerlLoader {
	GPluginLoader parent;
};

G_DEFINE_DYNAMIC_TYPE(
	GPluginPerlLoader,
	gplugin_perl_loader,
	GPLUGIN_TYPE_LOADER)

static PerlInterpreter *my_perl = NULL;

/******************************************************************************
 * Perl Stuff
 *****************************************************************************/
extern void boot_DynaLoader(pTHX_ CV *cv);

static void gplugin_perl_loader_xs_init(pTHX)
{
	dXSUB_SYS;

	newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, __FILE__);
}

static void
gplugin_perl_loader_init_perl(void)
{
	gchar *args[] = {
		"",
	};
	gchar **argv = (gchar **)args;
	gint argc = 1;

	PERL_SYS_INIT(&argc, &argv);

	my_perl = perl_alloc();
	PERL_SET_CONTEXT(my_perl);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
	perl_construct(my_perl);
}

static void
gplugin_perl_loader_uninit_perl(void)
{
	PERL_SET_CONTEXT(my_perl);
	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();
}

static GPluginPluginInfo *
gplugin_perl_loader_call_gplugin_query(
	PerlInterpreter *interpreter,
	GError **error)
{
	GPluginPluginInfo *info = NULL;
	PerlInterpreter *old = NULL;
	SV *err_tmp;
	gint ret = 0;

	dSP;

	old = my_perl;
	my_perl = interpreter;
	PERL_SET_CONTEXT(interpreter);

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);
	PUTBACK;

	ret = call_pv("gplugin_query", G_EVAL | G_NOARGS);

	SPAGAIN;

	/* ERRSV is a macro, so we store it instead of calling it multiple times. */
	err_tmp = ERRSV;
	if(SvTRUE(err_tmp)) {
		const gchar *errmsg = SvPVutf8_nolen(err_tmp);

		g_set_error_literal(error, GPLUGIN_DOMAIN, 0, errmsg);
	} else {
		if(ret != 1) {
			g_set_error_literal(
				error,
				GPLUGIN_DOMAIN,
				0,
				"gplugin_query did not return a GPluginPluginInfo");
		} else {
			info = (GPluginPluginInfo *)gperl_get_object(POPs);

			/* if we did get a real GPluginPluginInfo ref it because the perl
			 * code below will take it out of scope and delete it if its
			 * reference count is zero.
			 */
			if(GPLUGIN_IS_PLUGIN_INFO(info)) {
				g_object_ref(G_OBJECT(info));
			}
		}
	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	my_perl = old;

	return info;
}

/******************************************************************************
 * GPluginLoaderInterface API
 *****************************************************************************/
static GSList *
gplugin_perl_loader_supported_extensions(G_GNUC_UNUSED GPluginLoader *l)
{
	return g_slist_append(NULL, "pl");
}

static GPluginPlugin *
gplugin_perl_loader_query(
	GPluginLoader *loader,
	const gchar *filename,
	GError **error)
{
	GPluginPlugin *plugin = NULL;
	GPluginPluginInfo *info = NULL;
	PerlInterpreter *interpreter = NULL;
	const gchar *args[] = {"", filename};
	gchar **argv = (gchar **)args;
	gint argc = 2, ret = 0;

	interpreter = perl_alloc();
	PERL_SET_CONTEXT(interpreter);
	PL_perl_destruct_level = 1;
	perl_construct(interpreter);

	ret =
		perl_parse(interpreter, gplugin_perl_loader_xs_init, argc, argv, NULL);
	if(ret != 0) {
		SV *err_tmp;
		const gchar *errmsg = "unknown error";

		/* ERRSV is a macro so we need to store its returned value so we don't
		 * call it multiple times.
		 */
		err_tmp = ERRSV;
		if(SvTRUE(err_tmp)) {
			errmsg = SvPVutf8_nolen(err_tmp);
		}

		g_set_error_literal(error, GPLUGIN_DOMAIN, 0, errmsg);

		perl_destruct(interpreter);
		perl_free(interpreter);

		return NULL;
	}

	ret = perl_run(interpreter);
	if(ret != 0) {
		SV *err_tmp;
		const gchar *errmsg = "unknown error";

		/* ERRSV is a macro so we need to store its returned value so we don't
		 * call it multiple times.
		 */
		err_tmp = ERRSV;
		if(SvTRUE(err_tmp)) {
			errmsg = SvPVutf8_nolen(err_tmp);
		}

		g_set_error_literal(error, GPLUGIN_DOMAIN, 0, errmsg);

		perl_destruct(interpreter);
		perl_free(interpreter);

		return NULL;
	}

	info = gplugin_perl_loader_call_gplugin_query(interpreter, error);
	if(!GPLUGIN_IS_PLUGIN_INFO(info)) {
		/* If the plugin's query method didn't set error, set it to a generic
		 * message.
		 */
		if(error != NULL && *error == NULL) {
			g_set_error_literal(error, GPLUGIN_DOMAIN, 0, "failed to query");
		}

		return NULL;
	}

	/* clang-format off */
	plugin = g_object_new(
		GPLUGIN_PERL_TYPE_PLUGIN,
		"interpreter", interpreter,
		"filename", filename,
		"info", info,
		"loader", g_object_ref(loader),
		NULL);
	/* clang-format on */

	return plugin;
}

static gboolean
gplugin_perl_loader_load(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginPerlPlugin *pplugin = GPLUGIN_PERL_PLUGIN(plugin);
	PerlInterpreter *old = NULL;
	SV *err_tmp = NULL;
	gboolean r = FALSE;
	gint count = 0;

	dSP;

	old = my_perl;
	my_perl = gplugin_perl_plugin_get_interpreter(pplugin);
	PERL_SET_CONTEXT(my_perl);

	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	EXTEND(SP, 1);
	PUSHs(sv_2mortal(newSVGObject(g_object_ref(G_OBJECT(pplugin)))));

	PUTBACK;
	count = call_pv("gplugin_load", G_EVAL | G_SCALAR);
	SPAGAIN;

	/* ERRSV is a macro, so we store it instead of calling it multiple times. */
	err_tmp = ERRSV;
	if(SvTRUE(err_tmp)) {
		const gchar *errmsg = SvPVutf8_nolen(err_tmp);

		g_set_error_literal(error, GPLUGIN_DOMAIN, 0, errmsg);
	} else {
		if(count != 1) {
			g_set_error_literal(
				error,
				GPLUGIN_DOMAIN,
				0,
				"gplugin_load did not return a value");
		} else {
			if(POPi == 0) {
				r = TRUE;
			}
		}
	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	my_perl = old;

	PERL_SET_CONTEXT(my_perl);

	return r;
}

static gboolean
gplugin_perl_loader_unload(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	gboolean shutdown,
	GError **error)
{
	GPluginPerlPlugin *pplugin = GPLUGIN_PERL_PLUGIN(plugin);
	PerlInterpreter *old = NULL;
	SV *err_tmp = NULL;
	gboolean r = FALSE;
	gint count = 0;

	dSP;
	old = my_perl;
	my_perl = gplugin_perl_plugin_get_interpreter(pplugin);
	PERL_SET_CONTEXT(my_perl);

	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	EXTEND(SP, 2);
	PUSHs(sv_2mortal(newSVGObject(g_object_ref(G_OBJECT(pplugin)))));
	PUSHs(sv_2mortal(newSViv(shutdown)));

	PUTBACK;
	count = call_pv("gplugin_unload", G_EVAL | G_SCALAR);
	SPAGAIN;

	/* ERRSV is a macro, so we store it instead of calling it multiple times. */
	err_tmp = ERRSV;
	if(SvTRUE(err_tmp)) {
		const gchar *errmsg = SvPVutf8_nolen(err_tmp);

		g_set_error_literal(error, GPLUGIN_DOMAIN, 0, errmsg);
	} else {
		if(count != 1) {
			g_set_error_literal(
				error,
				GPLUGIN_DOMAIN,
				0,
				"gplugin_unload did not return a value");
		} else {
			if(POPi == 0) {
				r = TRUE;
			}
		}
	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	my_perl = old;
	PERL_SET_CONTEXT(my_perl);

	return r;
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
gplugin_perl_loader_init(G_GNUC_UNUSED GPluginPerlLoader *loader)
{
}

static void
gplugin_perl_loader_class_init(GPluginPerlLoaderClass *klass)
{
	GPluginLoaderClass *loader_class = GPLUGIN_LOADER_CLASS(klass);

	loader_class->supported_extensions =
		gplugin_perl_loader_supported_extensions;
	loader_class->query = gplugin_perl_loader_query;
	loader_class->load = gplugin_perl_loader_load;
	loader_class->unload = gplugin_perl_loader_unload;

	/* perl initialization */
	gplugin_perl_loader_init_perl();
}

static void
gplugin_perl_loader_class_finalize(G_GNUC_UNUSED GPluginPerlLoaderClass *klass)
{
	/* perl uninitialization */
	gplugin_perl_loader_uninit_perl();
}

/******************************************************************************
 * API
 *****************************************************************************/
void
gplugin_perl_loader_register(GTypeModule *module)
{
	gplugin_perl_loader_register_type(module);
}

GPluginLoader *
gplugin_perl_loader_new(void)
{
	/* clang-format off */
	return g_object_new(
		GPLUGIN_PERL_TYPE_LOADER,
		"id", "gplugin-perl5",
		NULL);
	/* clang-format on */
}
