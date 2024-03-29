/*
 * pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <locale.h>

#include <purple.h>

#include "gtkaccount.h"
#include "gtkblist.h"
#include "gtkconn.h"
#include "gtkxfer.h"
#include "gtkidle.h"
#include "gtkmedia.h"
#include "gtknotify.h"
#include "gtkprivacy.h"
#include "gtkrequest.h"
#include "gtkroomlist.h"
#include "gtkutils.h"
#include "gtkwhiteboard.h"
#include "pidginapplication.h"
#include "pidgincore.h"
#include "pidgindebug.h"
#include "pidginplugininfo.h"
#include "pidginprefs.h"
#include "pidginprivate.h"

#ifndef _WIN32
#include <signal.h>
#endif

#ifndef _WIN32

/*
 * Lists of signals we wish to catch and those we wish to ignore.
 * Each list terminated with -1
 */
static const int catch_sig_list[] = {
	SIGSEGV,
	SIGINT,
	SIGTERM,
	SIGQUIT,
	SIGCHLD,
	-1
};

static const int ignore_sig_list[] = {
	SIGPIPE,
	-1
};
#endif /* !_WIN32 */

#ifndef _WIN32
static char *segfault_message;

static guint signal_channel_watcher;

static int signal_sockets[2];

static void sighandler(int sig);

static void sighandler(int sig)
{
	ssize_t written;

	/*
	 * We won't do any of the heavy lifting for the signal handling here
	 * because we have no idea what was interrupted.  Previously this signal
	 * handler could result in some calls to malloc/free, which can cause
	 * deadlock in libc when the signal handler was interrupting a previous
	 * malloc or free.  So instead we'll do an ugly hack where we write the
	 * signal number to one end of a socket pair.  The other half of the
	 * socket pair is watched by our main loop.  When the main loop sees new
	 * data on the socket it reads in the signal and performs the appropriate
	 * action without fear of interrupting stuff.
	 */
	if (sig == SIGSEGV) {
		fprintf(stderr, "%s", segfault_message);
		abort();
		return;
	}

	written = write(signal_sockets[0], &sig, sizeof(int));
	if (written != sizeof(int)) {
		/* This should never happen */
		purple_debug_error("sighandler", "Received signal %d but only "
				"wrote %" G_GSSIZE_FORMAT " bytes out of %"
				G_GSIZE_FORMAT ": %s\n",
				sig, written, sizeof(int), g_strerror(errno));
		exit(1);
	}
}

static gboolean
mainloop_sighandler(GIOChannel *source, GIOCondition cond, gpointer data)
{
	GIOStatus stat;
	int sig;
	gsize bytes_read;
	GError *error = NULL;

	/* read the signal number off of the io channel */
	stat = g_io_channel_read_chars(source, (gchar *)&sig, sizeof(int),
			&bytes_read, &error);
	if (stat != G_IO_STATUS_NORMAL) {
		purple_debug_error("sighandler", "Signal callback failed to read "
				"from signal socket: %s", error->message);
		purple_core_quit();
		return FALSE;
	}

	switch (sig) {
		case SIGCHLD:
			/* Restore signal catching */
			signal(SIGCHLD, sighandler);
			break;
		default:
			purple_debug_warning("sighandler", "Caught signal %d\n", sig);
			purple_core_quit();
	}

	return TRUE;
}
#endif /* !_WIN32 */

static void
purple_ui_add_protocol_theme_paths(PurpleProtocol *protocol) {
	GdkDisplay *display = NULL;
	GtkIconTheme *theme = NULL;
	const gchar *path = NULL;

	display = gdk_display_get_default();

	theme = gtk_icon_theme_get_for_display(display);

	path = purple_protocol_get_icon_search_path(protocol);
	if(path != NULL) {
		gtk_icon_theme_add_search_path(theme, path);
	}

	path = purple_protocol_get_icon_resource_path(protocol);
	if(path != NULL) {
		gtk_icon_theme_add_resource_path(theme, path);
	}
}

static void
purple_ui_protocol_foreach_theme_cb(PurpleProtocol *protocol, gpointer data) {
	purple_ui_add_protocol_theme_paths(protocol);
}

static void
purple_ui_protocol_registered_cb(PurpleProtocolManager *manager,
                                 PurpleProtocol *protocol)
{
	purple_ui_add_protocol_theme_paths(protocol);
}

static gboolean
pidgin_history_init(GError **error) {
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	gchar *filename = NULL;
	const gchar *id = NULL;

	manager = purple_history_manager_get_default();

	/* Attempt to create the config_dir. We don't care about the result as the
	 * logging adapter will fail with a better error than us failing to create
	 * the directory.
	 */
	g_mkdir_with_parents(purple_config_dir(), 0700);

	filename = g_build_filename(purple_config_dir(), "history.db", NULL);
	adapter = purple_sqlite_history_adapter_new(filename);
	g_free(filename);

	id = purple_history_adapter_get_id(adapter);
	if(!purple_history_manager_register(manager, adapter, error)) {
		g_clear_object(&adapter);

		return FALSE;
	}

	/* The manager adds a ref to the adapter on registration, so we can remove
	 * our reference.
	 */
	g_clear_object(&adapter);

	return purple_history_manager_set_active(manager, id, error);
}

static void
pidgin_ui_init(void)
{
	PurpleProtocolManager *protocol_manager = NULL;
	GdkDisplay *display = NULL;
	GtkIconTheme *theme = NULL;
	GError *error = NULL;
	gchar *path;

	pidgin_debug_init();

	display = gdk_display_get_default();
	theme = gtk_icon_theme_get_for_display(display);

	path = g_build_filename(PURPLE_DATADIR, "pidgin", "icons", NULL);
	gtk_icon_theme_add_search_path(theme, path);
	g_free(path);

	/* Add a callback for when a protocol is registered to add its icon paths
	 * if it was found after initial startup.
	 */
	protocol_manager = purple_protocol_manager_get_default();
	g_signal_connect(protocol_manager, "registered",
	                 G_CALLBACK(purple_ui_protocol_registered_cb), NULL);

	/* Add the icon paths for all the protocols that libpurple found at start
	 * up.
	 */
	purple_protocol_manager_foreach(protocol_manager,
	                                purple_ui_protocol_foreach_theme_cb, NULL);

	if(!pidgin_history_init(&error)) {
		g_critical("failed to initialize the history api: %s",
		           error != NULL ? error->message : "unknown");
		g_clear_error(&error);
	}

	/* Set the UI operation structures. */
	purple_xfers_set_ui_ops(pidgin_xfers_get_ui_ops());
	purple_blist_set_ui(PIDGIN_TYPE_BUDDY_LIST);
	purple_notify_set_ui_ops(pidgin_notify_get_ui_ops());
	purple_request_set_ui_ops(pidgin_request_get_ui_ops());
	purple_connections_set_ui_ops(pidgin_connections_get_ui_ops());
	purple_whiteboard_set_ui_ops(pidgin_whiteboard_get_ui_ops());
	purple_idle_set_ui(pidgin_idle_new());

	pidgin_accounts_init();
	pidgin_connection_init();
	pidgin_request_init();
	pidgin_blist_init();
	pidgin_conversations_init();
	pidgin_commands_init();
	pidgin_privacy_init();
	pidgin_xfers_init();
	pidgin_roomlist_init();
	pidgin_medias_init();
	pidgin_notify_init();
}

static void
pidgin_quit(void)
{
	/* Uninit */

	/* Be sure to close all windows that are not attached to anything
	 * (e.g., the debug window), or they may access things after they are
	 * shut down. */
	pidgin_notify_uninit();
	pidgin_commands_uninit();
	pidgin_conversations_uninit();
	pidgin_blist_uninit();
	pidgin_request_uninit();
	pidgin_connection_uninit();
	pidgin_accounts_uninit();
	pidgin_xfers_uninit();
	pidgin_debug_window_hide();
	pidgin_debug_uninit();

	/* and end it all... */
	g_application_quit(g_application_get_default());
}

static PurpleCoreUiOps core_ops = {
	.ui_prefs_init = pidgin_prefs_init,
	.ui_init = pidgin_ui_init,
	.quit = pidgin_quit,
};

PurpleCoreUiOps *
pidgin_core_get_ui_ops(void)
{
	return &core_ops;
}

#ifndef _WIN32
static void
pidgin_setup_error_handler(void)
{
	int sig_indx;	/* for setting up signal catching */
	sigset_t sigset;
	char errmsg[BUFSIZ];
	GIOChannel *signal_channel;
	GIOStatus signal_status;
	GError *error = NULL;
	char *segfault_message_tmp;

	/* We translate this here in case the crash breaks gettext. */
	segfault_message_tmp = g_strdup_printf(_(
		"%s %s has segfaulted and attempted to dump a core file.\n"
		"This is a bug in the software and has happened through\n"
		"no fault of your own.\n\n"
		"If you can reproduce the crash, please notify the developers\n"
		"by reporting a bug at:\n"
		"%snewissue\n\n"
		"Please make sure to specify what you were doing at the time\n"
		"and post the backtrace from the core file.  If you do not know\n"
		"how to get the backtrace, please read the instructions at\n"
		"https://developer.pidgin.im/wiki/GetABacktrace\n"),
		PIDGIN_NAME, DISPLAY_VERSION, PURPLE_WEBSITE
	);

	/* we have to convert the message (UTF-8 to console
	   charset) early because after a segmentation fault
	   it's not a good practice to allocate memory */
	segfault_message = g_locale_from_utf8(segfault_message_tmp, -1, NULL, NULL,
	                                      &error);
	if(segfault_message != NULL) {
		g_free(segfault_message_tmp);
	} else {
		/* use 'segfault_message_tmp' (UTF-8) as a fallback */
		g_warning("%s\n", error->message);
		g_clear_error(&error);
		segfault_message = segfault_message_tmp;
	}

	/*
	 * Create a socket pair for receiving unix signals from a signal
	 * handler.
	 */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, signal_sockets) < 0) {
		perror("Failed to create sockets for GLib signal handling");
		exit(1);
	}
	signal_channel = g_io_channel_unix_new(signal_sockets[1]);

	/*
	 * Set the channel encoding to raw binary instead of the default of
	 * UTF-8, because we'll be sending integers across instead of strings.
	 */
	signal_status = g_io_channel_set_encoding(signal_channel, NULL, &error);
	if (signal_status != G_IO_STATUS_NORMAL) {
		fprintf(stderr, "Failed to set the signal channel to raw "
				"binary: %s", error->message);
		g_clear_error(&error);
		exit(1);
	}
	signal_channel_watcher = g_io_add_watch(signal_channel, G_IO_IN, mainloop_sighandler, NULL);
	g_io_channel_unref(signal_channel);

	/* Let's not violate any PLA's!!!! */
	/* jseymour: whatever the fsck that means */
	/* Robot101: for some reason things like gdm like to block     *
	 * useful signals like SIGCHLD, so we unblock all the ones we  *
	 * declare a handler for. thanks JSeymour and Vann.            */
	if (sigemptyset(&sigset)) {
		g_snprintf(errmsg, sizeof(errmsg),
		           "Warning: couldn't initialise empty signal set");
		perror(errmsg);
	}
	for(sig_indx = 0; catch_sig_list[sig_indx] != -1; ++sig_indx) {
		if(signal(catch_sig_list[sig_indx], sighandler) == SIG_ERR) {
			g_snprintf(errmsg, sizeof(errmsg),
			           "Warning: couldn't set signal %d for catching",
			           catch_sig_list[sig_indx]);
			perror(errmsg);
		}
		if(sigaddset(&sigset, catch_sig_list[sig_indx])) {
			g_snprintf(errmsg, sizeof(errmsg),
			           "Warning: couldn't include signal %d for unblocking",
			           catch_sig_list[sig_indx]);
			perror(errmsg);
		}
	}
	for(sig_indx = 0; ignore_sig_list[sig_indx] != -1; ++sig_indx) {
		if(signal(ignore_sig_list[sig_indx], SIG_IGN) == SIG_ERR) {
			g_snprintf(errmsg, sizeof(errmsg),
			           "Warning: couldn't set signal %d to ignore",
			           ignore_sig_list[sig_indx]);
			perror(errmsg);
		}
	}

	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL)) {
		g_snprintf(errmsg, sizeof(errmsg), "Warning: couldn't unblock signals");
		perror(errmsg);
	}
}
#endif /* !_WIN32 */

int pidgin_start(int argc, char *argv[])
{
	GApplication *app;
	int ret;

	bindtextdomain(PACKAGE, PURPLE_LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	/* Locale initialization is not complete here.  See gtk_init_check() */
	setlocale(LC_ALL, "");

#ifndef _WIN32
	pidgin_setup_error_handler();
#endif

	app = pidgin_application_new();
	g_application_set_default(app);

	ret = g_application_run(app, argc, argv);

	/* Make sure purple has quit in case something in GApplication
	 * has caused g_application_run() to finish on its own. This can
	 * happen, for example, if the desktop session is ending.
	 */
	if (purple_get_core() != NULL) {
		purple_core_quit();
	}

	if (g_application_get_is_registered(app) &&
			g_application_get_is_remote(app)) {
		g_printerr("%s\n", _("Exiting because another libpurple client is "
		                     "already running."));
	}

	/* Now that we're sure purple_core_quit() has been called,
	 * this can be freed.
	 */
	g_object_unref(app);

#ifndef _WIN32
	g_free(segfault_message);
	g_source_remove(signal_channel_watcher);
	close(signal_sockets[0]);
	close(signal_sockets[1]);
#endif

#ifdef _WIN32
	winpidgin_cleanup();
#endif

	return ret;
}
