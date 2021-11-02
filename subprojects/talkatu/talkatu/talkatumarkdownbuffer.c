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

#include <cmark.h>

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatubuffer.h"
#include "talkatu/talkatucodeset.h"
#include "talkatu/talkatumarkdownbuffer.h"
#include "talkatu/talkatutag.h"
#include "talkatu/talkatutagtable.h"

/**
 * SECTION:talkatumarkdownbuffer
 * @Title: Markdown Buffer
 * @Short_description: Markdown support
 *
 * Support for Markdown is provided via a custom GtkTextBuffer that knows how
 * to render it properly.
 */

/**
 * TALKATU_TYPE_MARKDOWN_BUFFER:
 *
 * The standard _get_type macro for #TalkatuMarkdownBuffer.
 */

/**
 * TalkatuMarkdownBuffer:
 *
 * A #TalkatuBuffer subclass that renders markdown.
 */

/**
 * TalkatuMarkdownBufferClass:
 * @insert_markdown: The insert_markdown vfunc is called to render
 *                   markdown into the #TalkatuMarkdownBuffer.
 *
 * The backing class for #TalkatuMarkdownBuffer.
 */

#define TALKATU_MARKDOWN_LINK_TARGET_ATTRIBUTE "markdown_link_target"

G_DEFINE_TYPE(TalkatuMarkdownBuffer, talkatu_markdown_buffer, TALKATU_TYPE_BUFFER);

/******************************************************************************
 * Helpers
 *****************************************************************************/
gboolean
talkatu_markdown_buffer_deserialize_markdown(GtkTextBuffer *register_buffer,
                                             GtkTextBuffer *content_buffer,
                                             GtkTextIter *iter,
                                             const guint8 *data,
                                             gsize length,
                                             gboolean create_tags,
                                             gpointer user_data,
                                             GError **error)
{
	GError *real_error = NULL;
	gchar *text = NULL;
	gsize text_length;

	text = talkatu_codeset_coerce_utf8(data, length, &text_length, &real_error);
	if(real_error) {
		if(error) {
			*error = real_error;
		}

		return FALSE;
	}

	talkatu_markdown_buffer_insert_markdown(
		TALKATU_MARKDOWN_BUFFER(register_buffer),
		iter,
		text,
		text_length
	);

	g_free(text);

	return TRUE;
}

static const gchar *
talkatu_markdown_buffer_get_tag_for_heading(cmark_node *node) {
	const gchar *tag_name = NULL;

	switch(cmark_node_get_heading_level(node)) {
		case 2:
			tag_name = TALKATU_TAG_H2;
			break;
		case 3:
			tag_name = TALKATU_TAG_H3;
			break;
		case 4:
			tag_name = TALKATU_TAG_H4;
			break;
		case 5:
			tag_name = TALKATU_TAG_H5;
			break;
		case 6:
			tag_name = TALKATU_TAG_H6;
			break;
		case 1:
		default:
			tag_name = TALKATU_TAG_H1;
			break;
	}

	return tag_name;
}

/******************************************************************************
 * TalkatuMarkdownBuffer Stuff
 *****************************************************************************/
static void
talkatu_markdown_buffer_real_insert_markdown(TalkatuMarkdownBuffer *buffer,
                                             GtkTextIter *iter,
                                             const gchar *text,
                                             gint len)
{
	GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer));
	gint start_offset = -1;
	cmark_node *root = NULL;
	cmark_iter *mditer = NULL;
	cmark_event_type ev_type;

	root = cmark_parse_document(text, len, CMARK_OPT_HARDBREAKS | CMARK_OPT_SAFE);
	mditer = cmark_iter_new(root);

	while((ev_type = cmark_iter_next(mditer)) != CMARK_EVENT_DONE) {
		cmark_node *node = cmark_iter_get_node(mditer);
		cmark_node_type node_type = cmark_node_get_type(node);

		if(node_type == CMARK_NODE_TEXT) {
			talkatu_buffer_insert_markup(
				TALKATU_BUFFER(buffer),
				iter,
				cmark_node_get_literal(node),
				-1
			);
		} else if(node_type == CMARK_NODE_CODE || node_type == CMARK_NODE_CODE_BLOCK) {
			talkatu_buffer_insert_markup_with_tags_by_name(
				TALKATU_BUFFER(buffer),
				iter,
				cmark_node_get_literal(node),
				-1,
				TALKATU_TAG_CODE,
				NULL
			);
		} else if(ev_type == CMARK_EVENT_ENTER) {
			start_offset = gtk_text_iter_get_offset(iter);
			g_message("enter node %s", cmark_node_get_type_string(node));
		} else if(ev_type == CMARK_EVENT_EXIT) {
			GSList *tags = NULL, *tag = NULL;
			const gchar *tag_name = NULL;

			if(start_offset == -1) {
				continue;
			}

			if(node_type == CMARK_NODE_EMPH) {
				tag_name = TALKATU_TAG_ITALIC;
			} else if(node_type == CMARK_NODE_STRONG) {
				tag_name = TALKATU_TAG_BOLD;
			} else if(node_type == CMARK_NODE_HEADING) {
				tag_name = talkatu_markdown_buffer_get_tag_for_heading(node);
			} else if(node_type == CMARK_NODE_LINK) {
				GtkTextTag *link_url = gtk_text_tag_new(NULL);

				tag_name = TALKATU_TAG_ANCHOR;

				g_object_set(
					G_OBJECT(link_url),
					TALKATU_MARKDOWN_LINK_TARGET_ATTRIBUTE,
					g_strdup(cmark_node_get_url(node)),
					NULL
				);

				gtk_text_tag_table_add(tag_table, link_url);

				g_message("link url: %s", cmark_node_get_url(node));

				tags = g_slist_append(tags, link_url);
			} else if(node_type == CMARK_NODE_PARAGRAPH) {
				talkatu_buffer_insert_markup(
					TALKATU_BUFFER(buffer),
					iter,
					"\n",
					-1
				);
			} else {
				g_warning("unknown node type %s", cmark_node_get_type_string(node));
			}

			if(tag_name || tags != NULL) {
				GtkTextIter start_iter;

				gtk_text_buffer_get_iter_at_offset(
					GTK_TEXT_BUFFER(buffer),
					&start_iter,
					start_offset
				);

				if(tag_name) {
					gtk_text_buffer_apply_tag_by_name(
						GTK_TEXT_BUFFER(buffer),
						tag_name,
						&start_iter,
						iter
					);
				}

				if(tags) {
					for(tag = tags; tag; tag = tag->next) {
						gtk_text_buffer_apply_tag(
							GTK_TEXT_BUFFER(buffer),
							GTK_TEXT_TAG(tag->data),
							&start_iter,
							iter
						);
					}

					g_slist_free(tags);
				}
			}

			/* now that we're done applying tags, if we were in a block element
			 * add a newline.
			 */
			g_message("exit node %s", cmark_node_get_type_string(node));
			if(node_type >= CMARK_NODE_FIRST_BLOCK && node_type <= CMARK_NODE_LAST_BLOCK) {
				g_message("finishing block node %s", cmark_node_get_type_string(node));
				talkatu_buffer_insert_markup(
					TALKATU_BUFFER(buffer),
					iter,
					"\n",
					-1
				);
			}

			start_offset = -1;
		}
	}

	cmark_iter_free(mditer);
	cmark_node_free(root);
}

/******************************************************************************
 * TalkatuBuffer Stuff
 *****************************************************************************/
static GSimpleActionGroup *
talkatu_markdown_buffer_create_action_group(TalkatuBuffer *buffer) {
	GSimpleActionGroup *ag = talkatu_action_group_new(GTK_TEXT_BUFFER(buffer));

	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_BOLD);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_ITALIC);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_UNDERLINE);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_STRIKETHROUGH);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_RESET);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_ATTACH_FILE);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_INSERT_LINK);

	return ag;
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_markdown_buffer_init(TalkatuMarkdownBuffer *buffer) {
	gtk_text_buffer_register_deserialize_format(
		GTK_TEXT_BUFFER(buffer),
		"text/markdown",
		talkatu_markdown_buffer_deserialize_markdown,
		NULL,
		NULL
	);
}

static void
talkatu_markdown_buffer_class_init(TalkatuMarkdownBufferClass *klass) {
	TalkatuBufferClass *buffer_class = TALKATU_BUFFER_CLASS(klass);

	buffer_class->create_action_group = talkatu_markdown_buffer_create_action_group;

	klass->insert_markdown = talkatu_markdown_buffer_real_insert_markdown;
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_markdown_buffer_new:
 *
 * Creates a new #TalkatuMarkdownBuffer that can render Markdown.
 *
 * Returns: (transfer full): The new #TalkatuMarkdownBuffer.
 */
GtkTextBuffer *talkatu_markdown_buffer_new(void) {
	return GTK_TEXT_BUFFER(g_object_new(
		TALKATU_TYPE_MARKDOWN_BUFFER,
		"tag-table", talkatu_tag_table_new(),
		NULL
	));
}

/**
 * talkatu_markdown_buffer_insert_markdown:
 * @buffer: The #TalkatuMarkdownBuffer instance.
 * @iter: The #GtkTextIter where the text should be inserted.
 * @text: The UTF-8 Markdown text that should be inserted.
 * @len: The length of @text or -1.
 *
 * Inserts and renders @text into @buffer.
 */
void
talkatu_markdown_buffer_insert_markdown(TalkatuMarkdownBuffer *buffer,
                                        GtkTextIter *iter,
                                        const gchar *text,
                                        gint len)
{
	TalkatuMarkdownBufferClass *klass = NULL;

	g_return_if_fail(TALKATU_IS_MARKDOWN_BUFFER(buffer));
	g_return_if_fail(iter != NULL);
	g_return_if_fail(text != NULL);

	klass = TALKATU_MARKDOWN_BUFFER_GET_CLASS(buffer);
	if(klass && klass->insert_markdown) {
		klass->insert_markdown(buffer, iter, text, len);
	}
}
