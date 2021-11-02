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

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gumbo.h>
#include <stdio.h>

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatuattachment.h"
#include "talkatu/talkatuattachmentdialog.h"
#include "talkatu/talkatubuffer.h"
#include "talkatu/talkatumarkup.h"
#include "talkatu/talkatutag.h"
#include "talkatu/talkatuview.h"

/**
 * SECTION:talkatuview
 * @Title: Text View
 * @Short_description: Widget that displays a TalkatuBuffer
 *
 * #TalkatuView handles the display of a #TalkatuBuffer, the keybindings for
 * applying formats, as well as displaying links.
 */

/**
 * TALKATU_TYPE_VIEW:
 *
 * The standard _get_type macro for #TalkatuView.
 */

/**
 * TalkatuViewClass:
 * @format_activate: The class handler for the #TalkatuView::format_activate
 *                   signal.
 * @open_url: The class handler for the #TalkatuView::open_url signal.
 *
 * The backing class to #TalkatuView instances.
 */

/**
 * TalkatuView:
 *
 * A #GtkTextView subclass that's preconfigured with a #TalkatuBuffer.
 */

typedef struct {
	GSimpleActionGroup *action_group;

	/* we cache the cursor that's displayed while hovering over a link as well
	 * the tag for links/anchors to avoid extra lookups.
	 */
	GdkCursor *cursor_hand;
	GtkTextTag *tag_anchor;
} TalkatuViewPrivate;

enum {
	SIG_FORMAT_ACTIVATE,
	SIG_OPEN_URL,
	LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = {0, };

G_DEFINE_TYPE_WITH_PRIVATE(TalkatuView, talkatu_view, GTK_TYPE_TEXT_VIEW)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gchar *
talkatu_view_url_from_iter(TalkatuView *view, GtkTextIter *iter) {
	GSList *tag = NULL;
	gchar *url = NULL;

	tag = gtk_text_iter_get_tags(iter);
	for(; tag != NULL; tag = g_slist_delete_link(tag, tag)) {
		if(tag->data == NULL) {
			continue;
		}

		url = g_object_get_data(G_OBJECT(tag->data), "talkatu-anchor-url");
		if(url != NULL) {
			break;
		}
	}
	g_slist_free(tag);

	return url;
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
talkatu_view_open_url_cb(GtkMenuItem *item, gpointer data) {
	TalkatuView *view = g_object_get_data(G_OBJECT(item), "view");
	gchar *url = g_object_get_data(G_OBJECT(item), "url");

	g_signal_emit(view, signals[SIG_OPEN_URL], 0, url);
}

static void
talkatu_view_copy_url_cb(GtkMenuItem *item, gpointer data) {
	GtkClipboard *clipboard = gtk_widget_get_clipboard(
		GTK_WIDGET(item),
		GDK_SELECTION_CLIPBOARD
	);

	gtk_clipboard_set_text(clipboard, (gchar *)data, -1);
}

gboolean
talkatu_view_anchor_tag_event_cb(GtkTextTag *tag,
                                 GObject *object,
                                 GdkEvent *event,
                                 GtkTextIter *iter,
                                 gpointer data)
{
	GdkEventType event_type = gdk_event_get_event_type(event);

	if(event_type == GDK_BUTTON_PRESS) {
		GdkEventButton *event_button = (GdkEventButton *)event;

		/* the user is right clicking on a link so stop the default handler */
		if(event_button->button == GDK_BUTTON_SECONDARY) {
			return TRUE;
		}
	} else if(gdk_event_triggers_context_menu(event)) {
		GdkEventButton *event_button = (GdkEventButton *)event;
		TalkatuView *view = TALKATU_VIEW(object);
		gchar *url;

		url = talkatu_view_url_from_iter(view, iter);

		/* if we didn't find a url, bail */
		if(url == NULL) {
			return FALSE;
		}

		if(event_button->button == GDK_BUTTON_PRIMARY) {
			GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

			/* old behavior from pidgin2, if the users has something selected
			 * we don't open links.  Other clients don't do this.. but it seems
			 * to be a work around for someone pressing a button over text,
			 * then moving the cursor to the link and then releasing.  Without
			 * the selection check that'll cause that to open the link if the
			 * button is released in the middle of the link.
			 */
			if(gtk_text_buffer_get_has_selection(buffer)) {
				return FALSE;
			}

			g_signal_emit(view, signals[SIG_OPEN_URL], 0, url);

			return TRUE;
		} else if(gdk_event_triggers_context_menu(event)) {
			GtkWidget *menu = gtk_menu_new();
			GtkWidget *item = NULL;

			item = gtk_menu_item_new_with_label(_("Open Link"));
			/* to make it easier to deal with the life cycle, we just add data
			 * to the menu item itself with destroy notifies.
			 */
			g_object_set_data_full(
				G_OBJECT(item),
				"view",
				g_object_ref(view),
				g_object_unref
			);
			g_object_set_data_full(
				G_OBJECT(item),
				"url",
				g_strdup(url),
				g_free
			);
			g_signal_connect(
				G_OBJECT(item),
				"activate",
				G_CALLBACK(talkatu_view_open_url_cb),
				NULL
			);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

			item = gtk_menu_item_new_with_label(_("Copy Link"));
			g_signal_connect(
				G_OBJECT(item),
				"activate",
				G_CALLBACK(talkatu_view_copy_url_cb),
				url
			);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

			gtk_widget_show_all(menu);
			G_GNUC_BEGIN_IGNORE_DEPRECATIONS
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, event_button->time);
			G_GNUC_END_IGNORE_DEPRECATIONS

			return TRUE;
		}
	}

	return FALSE;
}

static void
talkatu_view_buffer_set_cb(GObject *view, GParamSpec *pspec, gpointer data) {
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(TALKATU_VIEW(view));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	GtkTextTagTable *table = gtk_text_buffer_get_tag_table(buffer);

	if(TALKATU_IS_BUFFER(buffer)) {
		priv->action_group = talkatu_buffer_get_action_group(TALKATU_BUFFER(buffer));
	}

	/* check for the anchor tag, if we have it, add a signal handler to it */
	priv->tag_anchor = gtk_text_tag_table_lookup(table, TALKATU_TAG_ANCHOR);
	if(priv->tag_anchor != NULL) {
		g_signal_connect(
			priv->tag_anchor,
			"event",
			G_CALLBACK(talkatu_view_anchor_tag_event_cb),
			NULL
		);
	}
}

/******************************************************************************
 * Default Signal Handlers
 *****************************************************************************/
static void
talkatu_view_format_activate(TalkatuView *view, const gchar *action_name) {
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(view);

	if(priv->action_group) {
		GAction *action = g_action_map_lookup_action(G_ACTION_MAP(priv->action_group), action_name);

		if(action) {
			g_action_activate(action, NULL);
		}
	}
}

static gboolean
talkatu_view_query_tooltip(GtkWidget *widget,
                           gint x,
                           gint y,
                           gboolean keyboard,
                           GtkTooltip *tooltip)
{
	GtkTextIter iter;
	gchar *url = NULL;
	gint adj_x, adj_y;

	if(keyboard) {
		return GTK_WIDGET_CLASS(talkatu_view_parent_class)->query_tooltip(widget, x, y, keyboard, tooltip);
	}

	/* convert the window coordinates to match what's visible */
	gtk_text_view_window_to_buffer_coords(
		GTK_TEXT_VIEW(widget),
		GTK_TEXT_WINDOW_TEXT,
		x, y,
		&adj_x, &adj_y
	);

	/* now find the iter for what we're at */
	gtk_text_view_get_iter_at_location(
		GTK_TEXT_VIEW(widget),
		&iter,
		adj_x,
		adj_y
	);

	/* look for a url, if we have one, add it to tooltip */
	url = talkatu_view_url_from_iter(TALKATU_VIEW(widget), &iter);
	if(url != NULL) {
		gtk_tooltip_set_text(tooltip, url);

		return TRUE;
	}

	return GTK_WIDGET_CLASS(talkatu_view_parent_class)->query_tooltip(widget, x, y, keyboard, tooltip);
}

static gboolean
talkatu_view_motion_notify_event(GtkWidget *widget, GdkEventMotion *event) {
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(TALKATU_VIEW(widget));
	GtkTextIter iter;
	GdkCursor *cursor = NULL;
	gint x, y;

	gtk_text_view_window_to_buffer_coords(
		GTK_TEXT_VIEW(widget),
		GTK_TEXT_WINDOW_TEXT,
		event->x, event->y,
		&x, &y
	);

	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, x, y);

	if(gtk_text_iter_has_tag(&iter, priv->tag_anchor)) {
		cursor = priv->cursor_hand;
	}

	if(cursor != gdk_window_get_cursor(event->window)) {
		gdk_window_set_cursor(event->window, cursor);

		return TRUE;
	}

	return GTK_WIDGET_CLASS(talkatu_view_parent_class)->motion_notify_event(widget, event);
}

/******************************************************************************
 * GtkTextViewClass overrides
 *****************************************************************************/
static GtkTextBuffer *
talkatu_view_create_buffer(GtkTextView *view) {
	return talkatu_buffer_new(NULL);
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_view_finalize(GObject *obj) {
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(TALKATU_VIEW(obj));

	g_clear_object(&priv->cursor_hand);

	G_OBJECT_CLASS(talkatu_view_parent_class)->finalize(obj);
}

static void
talkatu_view_init(TalkatuView *view) {
	TalkatuViewPrivate *priv = talkatu_view_get_instance_private(view);

	priv->cursor_hand = gdk_cursor_new_from_name(gdk_display_get_default(), "pointer");

	/* tell the widget class that we support tooltips.  This is used to show
	 * link targets, and probably other stuff at some point.
	 */
	gtk_widget_set_has_tooltip(GTK_WIDGET(view), TRUE);

	/* set our event mask for the signals we care about */
	gtk_widget_set_events(
		GTK_WIDGET(view),
		GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
	);

	/* we need to know when the buffer is changed in our parent so we can
	 * update our actions and other stuff.
	 */
	g_signal_connect(
		G_OBJECT(view),
		"notify::buffer",
		G_CALLBACK(talkatu_view_buffer_set_cb),
		NULL
	);
}

static void
talkatu_view_class_init(TalkatuViewClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS(klass);
	GtkBindingSet *binding_set = NULL;

	obj_class->finalize = talkatu_view_finalize;

	widget_class->motion_notify_event = talkatu_view_motion_notify_event;
	widget_class->query_tooltip = talkatu_view_query_tooltip;

	text_view_class->create_buffer = talkatu_view_create_buffer;

	/* add our default signal handlers */
	klass->format_activate = talkatu_view_format_activate;

	/* add our signals */

	/**
	 * TalkatuView::format-activate
	 * @talkatutextview: The #TalkatuView instance.
	 * @arg1: The name of the action to activated.
	 * @user_data: User supplied data.
	 *
	 * Emitted by the keybindings to apply a format to the underlying buffer.
	 */
	signals[SIG_FORMAT_ACTIVATE] = g_signal_new(
		"format-activate",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(TalkatuViewClass, format_activate),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRING
	);

	/**
	 * TalkatuView::open-url:
	 * @talkatutextview: The #TalkatuView instances.
	 * @url: The URL to open.
	 * @user_data: User supplied data.
	 *
	 * Emitted when a user clicks on a link to open the url
	 */
	signals[SIG_OPEN_URL] = g_signal_new(
		"open-url",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(TalkatuViewClass, open_url),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRING
	);

	/* setup key bindings */
	binding_set = gtk_binding_set_by_class(talkatu_view_parent_class);

	/* remove existing keybindings that we're overriding */
	gtk_binding_entry_remove(binding_set, GDK_KEY_slash, GDK_CONTROL_MASK);

	/* add our custom keybindings */
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_b, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_BOLD);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_i, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_ITALIC);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_u, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_UNDERLINE);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_slash, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_STRIKETHROUGH);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_plus, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_GROW);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_equal, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_GROW);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_minus, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_SHRINK);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_r, GDK_CONTROL_MASK, "format-activate", 1, G_TYPE_STRING, TALKATU_ACTION_FORMAT_RESET);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_Insert, GDK_MOD1_MASK | GDK_SHIFT_MASK, "insert-at-cursor", 1, G_TYPE_STRING, "🐣");
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_view_new:
 *
 * Creates a new #TalkatuView with a #TalkatuBuffer.
 *
 * Returns: (transfer full): The new #TalkatuView.
 */
GtkWidget *talkatu_view_new(void) {
	return talkatu_view_new_with_buffer(talkatu_buffer_new(NULL));
}

/**
 * talkatu_view_new_with_buffer:
 * @buffer: A #GtkTextBuffer.
 *
 * Creates a new #TalkatuView with @buffer.
 *
 * Returns: (transfer full): The new #TalkatuView.
 */
GtkWidget *talkatu_view_new_with_buffer(GtkTextBuffer *buffer) {
	return GTK_WIDGET(g_object_new(
		TALKATU_TYPE_VIEW,
		"buffer", buffer,
		NULL));
}
