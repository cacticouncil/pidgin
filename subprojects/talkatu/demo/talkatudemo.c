/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <locale.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>

#include <talkatu/talkatu.h>

#include "talkatudemowindow.h"

static gboolean exit_early = FALSE;

static gboolean
talkatu_demo_version_cb(const gchar *n, const gchar *v, gpointer d, GError **e) {
	printf("talkatu-demo %s\n", TALKATU_VERSION);

	exit_early = TRUE;

	return TRUE;
}

static GOptionEntry entries[] = {
	{
		"version", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
		talkatu_demo_version_cb, N_("Display the version and exit"),
		NULL,
	}, {
		NULL, 0, 0, 0, NULL, NULL, NULL,
	}
};

gint
main(gint argc, gchar **argv) {
	GtkWidget *win = NULL;
	GError *error = NULL;
	GOptionContext *ctx = NULL;
	GOptionGroup *group = NULL;

	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	ctx = g_option_context_new(NULL);
	g_option_context_set_summary(ctx, _("Talkatu Demo"));
	g_option_context_set_translation_domain(ctx, GETTEXT_PACKAGE);
	g_option_context_add_main_entries(ctx, entries, NULL);

	group = gtk_get_option_group(FALSE);
	g_option_context_add_group(ctx, group);

	g_option_context_parse(ctx, &argc, &argv, &error);
	g_option_context_free(ctx);

	if(error) {
		fprintf(stderr, "%s\n", error->message);

		g_error_free(error);

		talkatu_uninit();

		return EXIT_FAILURE;
	}

	if(exit_early) {
		talkatu_uninit();

		return EXIT_SUCCESS;
	}

	gtk_init(&argc, &argv);

	talkatu_init();

	win = talkatu_demo_window_new();
	gtk_widget_show(win);

	gtk_main();

	talkatu_uninit();

	return 0;
}
