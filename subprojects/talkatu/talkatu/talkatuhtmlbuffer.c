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

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatubuffer.h"
#include "talkatu/talkatucodeset.h"
#include "talkatu/talkatuhtmlbuffer.h"
#include "talkatu/talkatumarkup.h"
#include "talkatu/talkatutag.h"
#include "talkatu/talkatutagtable.h"

/**
 * SECTION:talkatuhtmlbuffer
 * @Title: HTML Buffer
 * @Short_description: HTML support
 *
 * Support for HTML is provided via a custom GtkTextBuffer that knows how
 * to render it properly.
 */

/**
 * TALKATU_TYPE_HTML_BUFFER:
 *
 * The standard _get_type macro for #TalkatuHtmlBuffer.
 */

/**
 * TalkatuHtmlBuffer:
 *
 * A #TalkatuBuffer subclass that renders HTML.
 */

/**
 * TalkatuHtmlBufferClass:
 * @insert_html: The insert_html vfunc is called to render HTML into the
 *               #TalkatuHtmlBuffer.
 *
 * The backing class to #TalkatuHtmlBuffer.
 */

G_DEFINE_TYPE(TalkatuHtmlBuffer, talkatu_html_buffer, TALKATU_TYPE_BUFFER);

/******************************************************************************
 * TalkatuBuffer Stuff
 *****************************************************************************/
static GSimpleActionGroup *
talkatu_html_buffer_create_action_group(TalkatuBuffer *buffer) {
	GSimpleActionGroup *ag = talkatu_action_group_new(GTK_TEXT_BUFFER(buffer));

	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_BOLD);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_ITALIC);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_UNDERLINE);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_STRIKETHROUGH);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_SHRINK);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_GROW);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_RESET);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_ATTACH_FILE);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_INSERT_LINK);

	return ag;
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_html_buffer_init(TalkatuHtmlBuffer *buffer) {
	gtk_text_buffer_register_deserialize_format(
		GTK_TEXT_BUFFER(buffer),
		"text/html",
		talkatu_markup_deserialize_html,
		NULL,
		NULL
	);

	gtk_text_buffer_register_serialize_format(
		GTK_TEXT_BUFFER(buffer),
		"text/html",
		talkatu_markup_serialize_html,
		NULL,
		NULL
	);
}

static void
talkatu_html_buffer_class_init(TalkatuHtmlBufferClass *klass) {
	TalkatuBufferClass *buffer_class = TALKATU_BUFFER_CLASS(klass);

	buffer_class->create_action_group = talkatu_html_buffer_create_action_group;
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_html_buffer_new:
 *
 * Creates a new #TalkatuHtmlBuffer.  A #TalkatuHtmlBuffer supports HTML
 * formatting as well as pasting rendered HTML.
 *
 * Returns: (transfer full): The new #TalkatuHtmlBuffer instance.
 */
GtkTextBuffer *talkatu_html_buffer_new(void) {
	return GTK_TEXT_BUFFER(g_object_new(
		TALKATU_TYPE_HTML_BUFFER,
		"tag-table", talkatu_tag_table_new(),
		NULL
	));
}

/**
 * talkatu_html_buffer_insert_html:
 * @buffer: The #TalkatuHtmlBuffer instance.
 * @iter: The #GtkTextIter where to insert the HTML.
 * @text: The HTML text to insert.
 * @len: The length of @text or -1
 *
 * Inserts and renders @text as HTML into @buffer.
 */
void
talkatu_html_buffer_insert_html(TalkatuHtmlBuffer *buffer,
                                GtkTextIter *iter,
                                const gchar *text,
                                gint len)
{
	g_return_if_fail(TALKATU_IS_HTML_BUFFER(buffer));
	g_return_if_fail(iter != NULL);
	g_return_if_fail(text != NULL);

	talkatu_markup_insert_html(TALKATU_BUFFER(buffer), iter, text, len);
}
