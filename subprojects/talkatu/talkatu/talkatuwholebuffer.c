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

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatuwholebuffer.h"

/**
 * SECTION:talkatuwholebuffer
 * @Title: Whole Buffer
 * @Short_description: A whole format text buffer
 *
 * TalkatuWholeBuffer is a #GtkTextBuffer who's formatting applied to the
 * entire buffer and not on a character by character basis.
 */

/**
 * TALKATU_TYPE_WHOLE_BUFFER:
 *
 * The standard _get_type macro for #TalkatuWholeBuffer.
 */

/**
 * TalkatuWholeBuffer:
 *
 * A #TalkatuBuffer subclass that applies formatting to the entire buffer
 * rather that allowing rich formatting at character resolution.
 */
struct _TalkatuWholeBuffer {
	TalkatuBuffer parent;
};

G_DEFINE_TYPE(TalkatuWholeBuffer, talkatu_whole_buffer, TALKATU_TYPE_BUFFER)

/******************************************************************************
 * TalkatuBuffer Stuff
 *****************************************************************************/
static GSimpleActionGroup *
talkatu_whole_buffer_create_action_group(TalkatuBuffer *buffer) {
	GSimpleActionGroup *ag = talkatu_action_group_new(GTK_TEXT_BUFFER(buffer));

	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_BOLD);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_ITALIC);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_UNDERLINE);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_STRIKETHROUGH);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_SHRINK);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_GROW);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_RESET);
	talkatu_action_group_enable_action(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_INSERT_LINK);

	return ag;
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static GObject *
talkatu_whole_buffer_constructor(GType type,
                                 guint n_params,
                                 GObjectConstructParam *params)
{
	GObjectConstructParam *param;
	gint i = 0;

	for(i = 0, param = params; i < n_params; i++, param++) {
		const gchar *name = g_param_spec_get_name(param->pspec);
		if(g_ascii_strcasecmp("style", name) == 0) {
			g_value_set_enum(param->value, TALKATU_BUFFER_STYLE_WHOLE);
		}
	}

	return G_OBJECT_CLASS(talkatu_whole_buffer_parent_class)->constructor(type, n_params, params);
}

static void
talkatu_whole_buffer_init(TalkatuWholeBuffer *whole_buffer) {
}

static void
talkatu_whole_buffer_class_init(TalkatuWholeBufferClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	TalkatuBufferClass *buffer_class = TALKATU_BUFFER_CLASS(klass);

	obj_class->constructor = talkatu_whole_buffer_constructor;

	buffer_class->create_action_group = talkatu_whole_buffer_create_action_group;
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_whole_buffer_new:
 *
 * Creates a new #TalkatuWholeBuffer that applies formatting across the whole
 * text buffer instead of just a subset.
 *
 * Returns: (transfer full): The new #TalkatuWholeBuffer instance.
 */
GtkTextBuffer *talkatu_whole_buffer_new(void) {
	return GTK_TEXT_BUFFER(g_object_new(
		TALKATU_TYPE_WHOLE_BUFFER,
		"style", TALKATU_BUFFER_STYLE_WHOLE,
		NULL
	));
}
