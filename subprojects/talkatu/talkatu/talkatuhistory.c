/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include <talkatu/talkatuhistory.h>

#include <talkatu/talkatuattachmentpreview.h>
#include <talkatu/talkatuhistoryrow.h>
#include <talkatu/talkatumessage.h>

/**
 * SECTION:talkatuhistory
 * @Title: History View
 * @Short_description: Chat history widget
 *
 * This widget is used to display the conversation history.  It doesn't care if
 * it is multi user or one to one.
 */

/**
 * TALKATU_TYPE_HISTORY:
 *
 * The standard _get_type macro for #TalkatuHistory.
 */

/**
 * TalkatuHistory:
 *
 * A #TalkatuView subclass that is used to display a conversation.
 */
struct _TalkatuHistory {
	GtkListBox parent;
};

G_DEFINE_TYPE(TalkatuHistory, talkatu_history, GTK_TYPE_LIST_BOX)

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_history_init(TalkatuHistory *history) {
}

static void
talkatu_history_class_init(TalkatuHistoryClass *klass) {
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_history_new:
 *
 * Creates a new #TalkatuHistory for displaying chat history.
 *
 * Returns: (transfer full): The new #TalkatuHistory instance.
 */
GtkWidget *talkatu_history_new(void) {
	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_HISTORY,
		NULL
	));
}

/**
 * talkatu_history_write_message:
 * @history: The #TalkatuHistory instance.
 * @message: The #TalkatuMessage to add to @history.
 *
 * Adds @message to @history.  Messages are sorted by timestamp so make sure
 * it is set correctly.
 */
void
talkatu_history_write_message(TalkatuHistory *history,
                              TalkatuMessage *message)
{
	GtkWidget *row = NULL;

	g_return_if_fail(TALKATU_IS_HISTORY(history));
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	row = talkatu_history_row_new(message);
	gtk_container_add(GTK_CONTAINER(history), row);
}
