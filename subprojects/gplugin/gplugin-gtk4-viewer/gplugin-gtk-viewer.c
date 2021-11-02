/*
 * Copyright (C) 2021 Elliott Sales de Andrade <quantum.analyst@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gplugin.h>
#include <gplugin-gtk.h>

#include "gplugin-gtk-viewer-window.h"

/******************************************************************************
 * Globals
 *****************************************************************************/
static gboolean show_internal = FALSE;

/******************************************************************************
 * Helpers
 *****************************************************************************/

static gint
handle_local_options_cb(
	G_GNUC_UNUSED GApplication *app,
	GVariantDict *options,
	G_GNUC_UNUSED gpointer data)
{
	gboolean option = FALSE;

	if(g_variant_dict_lookup(options, "version", "b", &option, NULL)) {
		/* Handle --version and exit with success. */
		printf("gplugin-gtk-viewer %s\n", GPLUGIN_VERSION);

		return 0;
	}

	return -1;
}

static void
startup_cb(G_GNUC_UNUSED GApplication *app, G_GNUC_UNUSED gpointer data)
{
	GPluginManager *manager = NULL;
	const gchar *env_paths = NULL;

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);
	manager = gplugin_manager_get_default();

	env_paths = g_getenv("GPLUGIN_PLUGIN_PATH");
	if(env_paths != NULL) {
		gchar **paths = g_strsplit(env_paths, G_SEARCHPATH_SEPARATOR_S, 0);
		gint i;

		for(i = 0; paths[i] != NULL; i++) {
			gplugin_manager_prepend_path(manager, paths[i]);
		}

		g_strfreev(paths);
	}

	gplugin_manager_refresh(manager);
}

static void
shutdown_cb(G_GNUC_UNUSED GApplication *app, G_GNUC_UNUSED gpointer data)
{
	gplugin_uninit();
}

static void
activate_cb(GApplication *app)
{
	GtkWindow *window = NULL;

	g_return_if_fail(GTK_IS_APPLICATION(app));

	window = gtk_application_get_active_window(GTK_APPLICATION(app));
	if(window == NULL) {
		/* clang-format off */
		window = g_object_new(
			GPLUGIN_GTK_VIEWER_TYPE_WINDOW,
			"application", app,
			NULL);
		/* clang-format on */
	}

	gtk_window_present(window);
}

/******************************************************************************
 * Main Stuff
 *****************************************************************************/
/* clang-format off */
static GOptionEntry entries[] = {
	{
		"internal", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
		&show_internal, "Show internal plugins", NULL,
	}, {
		"version", 'V', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
		NULL, "Display the version and exit", NULL,
	}, {
		NULL, 0, 0, 0, NULL, NULL, NULL,
	},
};
/* clang-format on */

gint
main(gint argc, gchar **argv)
{
	GtkApplication *app = NULL;
	gint ret;

	/* Set up gettext translations. */
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	/* Prepare application and options. */
	app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
	g_application_add_main_option_entries(G_APPLICATION(app), entries);
	g_application_add_option_group(
		G_APPLICATION(app),
		gplugin_get_option_group());

	g_signal_connect(app, "activate", G_CALLBACK(activate_cb), NULL);
	g_signal_connect(
		app,
		"handle-local-options",
		G_CALLBACK(handle_local_options_cb),
		NULL);
	g_signal_connect(app, "startup", G_CALLBACK(startup_cb), NULL);
	g_signal_connect(app, "shutdown", G_CALLBACK(shutdown_cb), NULL);

	/* Run and get exit code. */
	ret = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return ret;
}
