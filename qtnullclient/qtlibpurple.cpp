//==============================================================================
//
// Filename: qtlibpurple.cpp
//
// File purpose: Definitions for functions that make direct calls to libpurple
// functions.
// 
//==============================================================================

#include <QApplication>
#include <QLabel>
#include <QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>

#include <purple.h>			// core (libpurple) functions
#include <defines.h>		// needed definitions
#include <glib.h>
#include <signal.h>

#include <string.h>
#include <iostream>

#ifndef _WIN32
#include <unistd.h>
#else
#include <win32/win32dep.h>
#endif

#include "qtlibpurple.h"


typedef struct _PurpleGLibIOClosure {
	PurpleInputFunction function;
	guint result;
	gpointer data;
} PurpleGLibIOClosure;

static void purple_glib_io_destroy(gpointer data)
{
	g_free(data);
}


static gboolean purple_glib_io_invoke(GIOChannel *source,
	GIOCondition condition, gpointer data)
{
	// In C++, a pointer of type void* may not be implicity converted to a
	// pointer of other type and so static_cast used to correct.
	PurpleGLibIOClosure *closure = static_cast<PurpleGLibIOClosure*>(data);

	// Static_cast used to correct invalid conversion from int to enum type
	// that was defined in libpurple/eventloop.h
	PurpleInputCondition purple_cond = static_cast<PurpleInputCondition>(0);

	// See above comments re: static_cast usage. Also, original bitwise
	// assignment operator statement was split into two statements to properly
	// implement static_cast. Note that double parentheses were used around OR
	// evaluation to avoid error message.
	if (condition & PURPLE_GLIB_READ_COND) {
		PurpleInputCondition temp_Purple_read_value = 
			static_cast<PurpleInputCondition>(PURPLE_INPUT_READ);
		purple_cond = static_cast<PurpleInputCondition>((purple_cond | 
			temp_Purple_read_value));
	}

	if (condition & PURPLE_GLIB_WRITE_COND) {
		PurpleInputCondition temp_Purple_write_value =
			static_cast<PurpleInputCondition>(PURPLE_INPUT_WRITE);
		purple_cond = static_cast<PurpleInputCondition>((purple_cond |
			temp_Purple_write_value));
	}

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
			  purple_cond);

	return TRUE;
}


static guint glib_input_add(gint fd, PurpleInputCondition condition,
	PurpleInputFunction function, gpointer data)
{
	PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
	GIOChannel *channel;
	
	// See above comments re: static_cast usage
	GIOCondition cond = static_cast<GIOCondition>(0);

	closure->function = function;
	closure->data = data;

	// See above comments re: static_cast usage and bitwise assignment operators
	if (condition & PURPLE_INPUT_READ)
	{
		GIOCondition temp_GIO_read_value =
			static_cast<GIOCondition>(PURPLE_GLIB_READ_COND);
		cond = static_cast<GIOCondition>((cond | temp_GIO_read_value));
	}

	if (condition & PURPLE_INPUT_WRITE) {
		GIOCondition temp_GIO_write_value =
			static_cast<GIOCondition>(PURPLE_GLIB_WRITE_COND);
		cond = static_cast<GIOCondition>((cond | temp_GIO_write_value));
	}

	#if defined _WIN32 && !defined WINPIDGIN_USE_GLIB_IO_CHANNEL
		channel = wpurple_g_io_channel_win32_new_socket(fd);
	#else
		channel = g_io_channel_unix_new(fd);
	#endif

	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
					      purple_glib_io_invoke, closure,
					      purple_glib_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}


// pidgin comments: Conversation uiops
static void null_write_conv(PurpleConversation *conv, const char *who,
	const char *alias, const char *message, PurpleMessageFlags flags,
	time_t mtime)
{
	const char *name;
	if (alias && *alias)
		name = alias;
	else if (who && *who)
		name = who;
	else
		name = NULL;

	printf("(%s) %s %s: %s\n", purple_conversation_get_name(conv),
			purple_utf8_strftime("(%H:%M:%S)", localtime(&mtime)),
			name, message);
}


static void null_ui_init(void)
{
	
	// pidgin comments: This should initialize the UI components for all the
	// modules. Here we just initialize the UI for conversations.
	purple_conversations_set_ui_ops(&null_conv_uiops);
}


void init_libpurple(void)
{
	// pidgin comments: Set a custom user directory (optional)
	purple_util_set_user_dir(CUSTOM_USER_DIRECTORY);

	// pidgin comments: We do not want any debugging for now to keep the noise
	// to a minimum.
	purple_debug_set_enabled(FALSE);

	// pidgin comments: Set the core-uiops, which is used to
	// 	- initialize the ui specific preferences.
	// 	- initialize the debug ui.
	// 	- initialize the ui components for all the modules.
	// 	- uninitialize the ui components for all the modules when the core
	//		terminates.
	purple_core_set_ui_ops(&null_core_uiops);

	// pidgin comments: Set the uiops for the eventloop. If your client is
	// glib-based, you can safely copy this verbatim.
	purple_eventloop_set_ui_ops(&glib_eventloops);

	// pidgin comments: Set path to search for plugins. The core (libpurple)
	// takes care of loading the core-plugins, which includes the
	// protocol-plugins. So it is not essential to add any path here, but it
	// might be desired, especially for ui-specific plugins.
	purple_plugins_add_search_path(CUSTOM_PLUGIN_PATH);

	// pidgin comments: Now that all the essential stuff has been set, let's
	// try to init the core. It's necessary to provide a non-NULL name for the
	// current ui to the core. This name is used by stuff that depends on this
	// ui, for example the ui-specific plugins.
	if (!purple_core_init(UI_ID))
	{
		// Initializing the core failed. Terminate.
		fprintf(stderr,
				"libpurple initialization failed. Dumping core.\n"
				"Please report this!\n");
		abort();
	}

	// pidgin comments: Create and load the buddylist.
	purple_set_blist(purple_blist_new());
	purple_blist_load();

	// pidgin comments: Load the preferences.
	purple_prefs_load();

	// pidgin comments: Load the desired plugins. The client should save the
	// list of loaded plugins in the preferences using
	// purple_plugins_save_loaded(PLUGIN_SAVE_PREF).
	purple_plugins_load_saved(PLUGIN_SAVE_PREF);

	// pidgin comments: Load the pounces.
	purple_pounces_load();
}


void signed_on(PurpleConnection *gc, gpointer null)
{
	PurpleAccount *account = purple_connection_get_account(gc);
	printf("Account connected: %s %s\n", account->username,
		account->protocol_id);

	(p->*function_ptr)(1);	
}


void connect_to_signals_for_demonstration_purposes_only(void)
{
	static int handle;
	purple_signal_connect(purple_connections_get_handle(), "signed-on",
		&handle, PURPLE_CALLBACK(signed_on), NULL);
}
