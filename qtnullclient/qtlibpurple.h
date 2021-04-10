//=============================================================================
//
// Filename: qtnullclient.h
//
// File purpose: Function declarations accompanying qtlibpurple.cpp
// 
//=============================================================================

#ifndef QTLIBPURPLE_H
#define QTLIBPURPLE_H

#include <purple.h>
#include "MainWindow.h"

// Global function pointer created in main.cpp
extern MainWindow* p;
extern void (MainWindow::*function_ptr)(int);


#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)


static void purple_glib_io_destroy(gpointer data);

static gboolean purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data);

static guint glib_input_add(gint fd, PurpleInputCondition condition, PurpleInputFunction function, gpointer data);


static PurpleEventLoopUiOps glib_eventloops =
{
	g_timeout_add,
	g_source_remove,
	glib_input_add,
	g_source_remove,
	NULL,

	#if GLIB_CHECK_VERSION(2,14,0)
		g_timeout_add_seconds,
	#else
		NULL,
	#endif

	// pidgin comments: padding
	NULL,
	NULL,
	NULL
};
// pidgin comments: End of the eventloop functions.


// pidgin comments: Conversation uiops.
static void null_write_conv(PurpleConversation *conv, const char *who, const char *alias, const char *message, PurpleMessageFlags flags, time_t mtime);


static PurpleConversationUiOps null_conv_uiops =
{
	NULL,                      // pidgin comments: create_conversation
	NULL,                      // pidgin comments: destroy_conversation
	NULL,                      // pidgin comments: write_chat
	NULL,                      // pidgin comments: write_im
	null_write_conv,           // pidgin comments: write_conv
	NULL,                      // pidgin comments: chat_add_users
	NULL,                      // pidgin comments: chat_rename_user
	NULL,                      // pidgin comments: chat_remove_users
	NULL,                      // pidgin comments: chat_update_user
	NULL,                      // pidgin comments: present
	NULL,                      // pidgin comments: has_focus
	NULL,                      // pidgin comments: custom_smiley_add
	NULL,                      // pidgin comments: custom_smiley_write
	NULL,                      // pidgin comments: custom_smiley_close
	NULL,                      // pidgin comments: send_confirm
	NULL,
	NULL,
	NULL,
	NULL
};


static void null_ui_init(void);


static PurpleCoreUiOps null_core_uiops =
{
	NULL,
	NULL,
	null_ui_init,
	NULL,

	// pidgin comments: padding
	NULL,
	NULL,
	NULL,
	NULL
};

// Original files were static functions but static label was removed to permit
// functions to be called from different source file.
void init_libpurple(void);
void signed_on(PurpleConnection *gc, gpointer null);
void connect_to_signals_for_demonstration_purposes_only(void);

#endif // QLIBPURPLE_H
