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

#include "talkatu/talkatumessageactions.h"

/**
 * SECTION:talkatumessageactions
 * @Title: Message actions
 * @Short_description: A display widget for #TalkatuMessage's
 *
 * Talkatu provides a simple formatting message_actions that gives the user the ability
 * to format any #TalkatuBuffer via the formatting actions.
 */

/**
 * TALKATU_TYPE_MESSAGE_ACTIONS:
 *
 * The standard _get_type macro for #TalkatuMessageActions.
 */

/**
 * TalkatuMessageActions:
 *
 * A composite #GtkWidget to allow the user to interact with a message in a
 * #TalkatuHistory.
 */
struct _TalkatuMessageActions {
	GtkBox parent;

	TalkatuMessage *message;
};

enum {
	PROP_0,
	PROP_MESSAGE,
	N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE(TalkatuMessageActions, talkatu_message_actions, GTK_TYPE_BOX)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
talkatu_message_actions_set_message(TalkatuMessageActions *message_actions,
                                    TalkatuMessage *message)
{
	if(message_actions->message) {
		g_object_unref(G_OBJECT(message_actions->message));
	}

	if(g_set_object(&message_actions->message, message)) {
		g_object_notify_by_pspec(G_OBJECT(message_actions), properties[PROP_MESSAGE]);
	}
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_message_actions_get_property(GObject *obj,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
	TalkatuMessageActions *message_actions = TALKATU_MESSAGE_ACTIONS(obj);

	switch(prop_id) {
		case PROP_MESSAGE:
			g_value_set_object(value, talkatu_message_actions_get_message(message_actions));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_message_actions_set_property(GObject *obj,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
	TalkatuMessageActions *message_actions = TALKATU_MESSAGE_ACTIONS(obj);

	switch(prop_id) {
		case PROP_MESSAGE:
			talkatu_message_actions_set_message(message_actions, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_message_actions_finalize(GObject *obj) {
	TalkatuMessageActions *message_actions = TALKATU_MESSAGE_ACTIONS(obj);

	g_object_unref(G_OBJECT(message_actions->message));

	G_OBJECT_CLASS(talkatu_message_actions_parent_class)->finalize(obj);
}

static void
talkatu_message_actions_init(TalkatuMessageActions *message_actions) {
	gtk_widget_init_template(GTK_WIDGET(message_actions));
}

static void
talkatu_message_actions_class_init(TalkatuMessageActionsClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = talkatu_message_actions_get_property;
	obj_class->set_property = talkatu_message_actions_set_property;
	obj_class->finalize = talkatu_message_actions_finalize;

	properties[PROP_MESSAGE] = g_param_spec_object(
		"message", "message", "The message that this widget is for",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
		GTK_WIDGET_CLASS(klass),
		"/org/imfreedom/keep/talkatu/talkatu/ui/messageactions.ui"
	);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_message_actions_new:
 * @message: The #TalkatuMessage instance.
 *
 * Creates a new #TalkatuMessageActions that provides the user with an interface to
 * control the formatting of a #TalkatuBuffer.
 *
 * Returns: (transfer full): The new #TalkatuMessageActions instance.
 */
GtkWidget *
talkatu_message_actions_new(TalkatuMessage *message) {
	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_MESSAGE_ACTIONS,
		"message", message,
		NULL
	));
}

/**
 * talkatu_message_actions_get_message:
 * @message_actions: The #TalkatuMessageActions instance.
 *
 * Gets the #TalkatuMessage associated with @message_action.
 *
 * Returns: (transfer none): The #TalkatuMessage associated with @message_action.
 */
TalkatuMessage *
talkatu_message_actions_get_message(TalkatuMessageActions *message_actions) {
	g_return_val_if_fail(TALKATU_IS_MESSAGE_ACTIONS(message_actions), NULL);

	return message_actions->message;
}
