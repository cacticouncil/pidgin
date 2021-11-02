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

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <gspell/gspell.h>

#include "talkatu/talkatuinput.h"

#include "talkatu/talkatuactiongroup.h"
#include "talkatu/talkatuattachmentdialog.h"
#include "talkatu/talkatuenums.h"
#include "talkatu/talkatumarkup.h"
#include "talkatu/talkatumessage.h"
#include "talkatu/talkatusimpleattachment.h"

/**
 * SECTION:talkatuinput
 * @Title: Input Widget
 * @Short_description: The main Talkatu input widget.
 *
 * #TalkatuInput handles all of the details for sending messages.
 */

/**
 * TALKATU_TYPE_INPUT:
 *
 * The standard _get_type macro for #TalkatuInput.
 */

/**
 * TalkatuInput:
 *
 * #TalkatuInput is the main input widget for Talkatu.  It supports WYSIWYG
 * input for both HTML and Markdown as well as plain text.
 *
 * It implements #TalkatuMessage which means it can be written directly to
 * #TalkatuHistory with talkatu_history_write_message().  That also means that
 * it can handle attachments.  Currently this is only supported programmatically.
 *
 * It provides keybinds for pasting images as well as emitting a signal when the
 * user has pressed a developer defined keybinding to "send" the message.
 */

/**
 * TalkatuInputClass:
 * @should_send_message: The class handler for the
 *                       #TalkatuInput::should_send_message signal.
 * @send_message: The class handler for the #TalkatuInput::send_message signal.
 *
 * The backing class to #TalkatuInput instances.
 */

/**
 * TalkatuInputSendBinding:
 * @TALKATU_INPUT_SEND_BINDING_RETURN: Represents return.
 * @TALKATU_INPUT_SEND_BINDING_KP_ENTER: Represents enter.
 * @TALKATU_INPUT_SEND_BINDING_SHIFT_RETURN: Represents shift-return.
 * @TALKATU_INPUT_SEND_BINDING_CONTROL_RETURN: Represents control-return.
 *
 * Flags for assigning and determining which key bindings should be used to
 * send a message.
 */

typedef struct {
	TalkatuView parent;

	GHashTable *attachments;
	guint64 attachment_id;

	GspellTextView *gspell_view;

	TalkatuInputSendBinding send_binding;

	/* this mark is used to keep track of our context for the context menu.  It
	 * is updated via cursor-moved and button-press callbacks.
	 */
	GtkTextMark *context_mark;

	/* TalkatuMessage properties: content type and contents are derived from
	 * the widget itself.
	 */
	gchar *id;
	GDateTime *timestamp;
	TalkatuContentType content_type;

	gchar *author;
	GdkRGBA *author_name_color;

	gboolean edited;
} TalkatuInputPrivate;

typedef struct {
	TalkatuAttachmentForeachFunc func;
	gpointer data;
} TalkatuInputForeachAttachmentData;

enum {
	PROP_0 = 0,
	PROP_SEND_BINDING,
	N_PROPERTIES,
	/* overrides */
	PROP_ID = N_PROPERTIES,
	PROP_TIMESTAMP,
	PROP_CONTENT_TYPE,
	PROP_AUTHOR,
	PROP_AUTHOR_NAME_COLOR,
	PROP_CONTENTS,
	PROP_EDITED,
};
static GParamSpec *properties[N_PROPERTIES];

enum {
	SIG_SHOULD_SEND_MESSAGE,
	SIG_SEND_MESSAGE,
	LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = {0, };

static void talkatu_input_message_init(TalkatuMessageInterface *iface);

G_DEFINE_TYPE_EXTENDED(
	TalkatuInput, talkatu_input, TALKATU_TYPE_VIEW,
	0,
	G_ADD_PRIVATE(TalkatuInput)
	G_IMPLEMENT_INTERFACE(TALKATU_TYPE_MESSAGE, talkatu_input_message_init)
);

/******************************************************************************
 * TalkatuMessage Interface
 *****************************************************************************/
static const gchar *
talkatu_input_get_id(TalkatuInput *input) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	return priv->id;
}

static void
talkatu_input_set_id(TalkatuInput *input, const gchar *id) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify(G_OBJECT(input), "id");
}

static GDateTime *
talkatu_input_get_timestamp(TalkatuInput *input) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	/* If the timestamp has been explicitly set, return that. */
	if(priv->timestamp != NULL) {
		return priv->timestamp;
	}

	/* If no timestamp has been explicitly set, just return the current local
	 * time.
	 */
	return g_date_time_new_now_local();
}

static void
talkatu_input_set_timestamp(TalkatuInput *input, GDateTime *timestamp) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	g_clear_pointer(&priv->timestamp, g_date_time_unref);
	if(timestamp != NULL) {
		priv->timestamp = g_date_time_ref(timestamp);
	}

	g_object_notify(G_OBJECT(input), "timestamp");
}

static TalkatuContentType
talkatu_input_get_content_type(TalkatuInput *input) {
	/* TODO: look at our buffer and map it */
	return TALKATU_CONTENT_TYPE_PLAIN;
}

static void
talkatu_input_set_content_type(TalkatuInput *input,
                               TalkatuContentType content_type)
{
	/* TODO: set the buffer here? */

	g_object_notify(G_OBJECT(input), "content-type");
}

static gchar *
talkatu_input_get_author(TalkatuInput *input) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	return priv->author;
}

static void
talkatu_input_set_author(TalkatuInput *input, const gchar *author) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	if(author == priv->author) {
		return;
	}

	g_clear_pointer(&priv->author, g_free);

	priv->author = g_strdup(author);

	g_object_notify(G_OBJECT(input), "author");
}

static GdkRGBA *
talkatu_input_get_author_name_color(TalkatuInput *input) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	if(priv->author_name_color != NULL) {
		return gdk_rgba_copy(priv->author_name_color);
	}

	return NULL;
}

static void
talkatu_input_set_author_name_color(TalkatuInput *input, GdkRGBA *color) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	g_clear_pointer(&priv->author_name_color, gdk_rgba_free);

	if(color != NULL) {
		priv->author_name_color = gdk_rgba_copy(color);
	}

	g_object_notify(G_OBJECT(input), "author-name-color");
}

static gchar *
talkatu_input_get_contents(TalkatuInput *input) {
	GtkTextBuffer *buffer = NULL;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));

	return talkatu_markup_get_html(buffer, NULL);
}

static void
talkatu_input_set_contents(TalkatuInput *input, const gchar *contents) {
	GtkTextBuffer *buffer = NULL;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));

	if(contents == NULL) {
		talkatu_buffer_clear(TALKATU_BUFFER(buffer));
	} else {
		talkatu_markup_set_html(TALKATU_BUFFER(buffer), contents, -1);
	}

	g_object_notify(G_OBJECT(input), "contents");
}

static gboolean
talkatu_input_get_edited(TalkatuInput *input) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	return priv->edited;
}

static void
talkatu_input_set_edited(TalkatuInput *input, gboolean edited) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	priv->edited = edited;

	g_object_notify(G_OBJECT(input), "edited");
}

static gboolean
talkatu_input_add_attachment(TalkatuMessage *message,
                             TalkatuAttachment *attachment)
{
	TalkatuInput *input = TALKATU_INPUT(message);
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	talkatu_attachment_set_id(attachment, priv->attachment_id++);

	return g_hash_table_insert(
		priv->attachments,
		talkatu_attachment_get_hash_key(attachment),
		g_object_ref(G_OBJECT(attachment))
	);
}

static gboolean
talkatu_input_remove_attachment(TalkatuMessage *message, guint64 id) {
	TalkatuInput *input = TALKATU_INPUT(message);
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	return g_hash_table_remove(priv->attachments, &id);
}

static TalkatuAttachment *
talkatu_input_get_attachment(TalkatuMessage *message, guint64 id) {
	TalkatuInput *input = TALKATU_INPUT(message);
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);
	TalkatuAttachment *attachment = NULL;

	attachment = g_hash_table_lookup(priv->attachments, &id);

	if(TALKATU_IS_ATTACHMENT(attachment)) {
		return TALKATU_ATTACHMENT(g_object_ref(G_OBJECT(attachment)));
	}

	return NULL;
}

static void
talkatu_input_foreach_attachment_helper(gpointer key, gpointer value,
                                        gpointer data)
{
	TalkatuInputForeachAttachmentData *d = (TalkatuInputForeachAttachmentData *)data;

	d->func(TALKATU_ATTACHMENT(value), d->data);
}

static void
talkatu_input_foreach_attachment(TalkatuMessage *message,
                                 TalkatuAttachmentForeachFunc func,
                                 gpointer data)
{
	TalkatuInput *input = TALKATU_INPUT(message);
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	TalkatuInputForeachAttachmentData d = {
		.func = func,
		.data = data
	};

	g_hash_table_foreach(priv->attachments,
	                     talkatu_input_foreach_attachment_helper,
	                     &d);
}

static void
talkatu_input_clear_attachments(TalkatuMessage *message) {
	TalkatuInput *input = TALKATU_INPUT(message);
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	g_hash_table_remove_all(priv->attachments);
	priv->attachment_id = 0;
}

static void
talkatu_input_message_init(TalkatuMessageInterface *iface) {
	iface->add_attachment = talkatu_input_add_attachment;
	iface->remove_attachment = talkatu_input_remove_attachment;
	iface->get_attachment = talkatu_input_get_attachment;
	iface->foreach_attachment = talkatu_input_foreach_attachment;
	iface->clear_attachments = talkatu_input_clear_attachments;
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
talkatu_input_send_message_cb(GtkMenuItem *item, gpointer data) {
	talkatu_input_send_message(TALKATU_INPUT(data));

	gtk_widget_grab_focus(GTK_WIDGET(data));
}


static void
talkatu_input_populate_popup_cb(GtkTextView *view, GtkWidget *popup) {
	TalkatuInputPrivate *priv = NULL;
	GtkTextBuffer *buffer = NULL;
	GtkTextIter iter;
	GtkWidget *item = NULL;
	gint pos = 0;

	/* if the popup isn't a menu, bail */
	if(!GTK_IS_MENU(popup)) {
		return;
	}

	priv = talkatu_input_get_instance_private(TALKATU_INPUT(view));

	buffer = gtk_text_view_get_buffer(view);

	gtk_text_buffer_get_iter_at_mark(buffer, &iter, priv->context_mark);

	/* add the send message item */
	if(gtk_text_view_get_editable(view)) {
		item = gtk_menu_item_new_with_label(_("Send message"));
		g_signal_connect_after(
			G_OBJECT(item),
			"activate",
			G_CALLBACK(talkatu_input_send_message_cb),
			view
		);
		gtk_menu_shell_insert(GTK_MENU_SHELL(popup), item, pos++);
		gtk_widget_show(item);

		item = gtk_separator_menu_item_new();
		gtk_menu_shell_insert(GTK_MENU_SHELL(popup), item , pos++);
		gtk_widget_show(item);
	}
}

static void
talkatu_input_buffer_set_cb(GObject *view, GParamSpec *pspec, gpointer data) {
	TalkatuInputPrivate *priv = NULL;
	TalkatuInput *input = TALKATU_INPUT(view);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	GtkTextIter start;

	priv = talkatu_input_get_instance_private(input);

	gspell_text_view_basic_setup(priv->gspell_view);

	/* grab our context_mark */
	gtk_text_buffer_get_start_iter(buffer, &start);
	priv->context_mark = gtk_text_buffer_create_mark(buffer, NULL, &start, TRUE);

	if(TALKATU_IS_BUFFER(buffer)) {
		GSimpleActionGroup *ag = NULL;

		ag = talkatu_buffer_get_action_group(TALKATU_BUFFER(buffer));

		if(TALKATU_IS_ACTION_GROUP(ag)) {
			/* Tell the action group of the buffer that we exist. */
			talkatu_action_group_set_input(TALKATU_ACTION_GROUP(ag), input);
		}
	}
}

static void
talkatu_input_attachment_response_cb(GtkDialog *dialog, gint response,
                                     gpointer data)
{
	/* If the user hits escape response is set to GTK_RESPONSE_DELETE_EVENT
	 * and Gtk cleans up the dialog for us automatically.
	 */

	if(response == GTK_RESPONSE_CANCEL) {
		/* we call this separately for GTK_RESPONSE_CANCEL because
		 * GTK_RESPONSE_DELETE_EVENT already destroys the dialog.
		 */
		gtk_widget_destroy(GTK_WIDGET(dialog));
	} else if(response == GTK_RESPONSE_ACCEPT) {
		GtkTextBuffer *buffer = NULL;
		TalkatuAttachment *attachment = NULL;
		TalkatuAttachmentDialog *adialog = TALKATU_ATTACHMENT_DIALOG(dialog);
		TalkatuInput *input = TALKATU_INPUT(data);
		const gchar *comment = NULL;
		gchar *escaped = NULL;

		comment = talkatu_attachment_dialog_get_comment(adialog);
		escaped = g_markup_escape_text(comment, -1);

		/* it's a pretty safe assumption that our buffer is a talkatu buffer */
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));
		talkatu_markup_set_html(TALKATU_BUFFER(buffer), escaped, -1);
		g_free(escaped);

		/* now send the attachment */
		attachment = talkatu_attachment_dialog_get_attachment(adialog);
		talkatu_message_add_attachment(TALKATU_MESSAGE(input), attachment);
		g_object_unref(G_OBJECT(attachment));

		/* send the message */
		talkatu_input_send_message(input);

		/* kill the dialog */
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}
}

static void
talkatu_input_image_received_cb(GtkClipboard *clipboard, GdkPixbuf *pixbuf,
                                gpointer data)
{
	TalkatuAttachment *attachment = NULL;
	GFile *file = NULL;
	GFileIOStream *stream = NULL;
	GOutputStream *ostream = NULL;
	GtkWidget *dialog = NULL, *input = NULL;
	GtkTextBuffer *buffer = NULL;
	GError *error = NULL;
	gchar *comment = NULL, *filename = NULL;
	gboolean saved = FALSE;
	guint64 size;

	/* save the image on clipboard to a temp file so we can reference it in the
	 * attachment.
	 */
	file = g_file_new_tmp("talkatu-clipboard-XXXXXX.png", &stream, &error);
	if(error != NULL) {
		g_warning("failed to create temp file: %s", error->message);
		g_error_free(error);

		return;
	}

	ostream = g_io_stream_get_output_stream(G_IO_STREAM(stream));

	saved = gdk_pixbuf_save_to_stream(pixbuf, ostream, "png", NULL, &error,
	                                  NULL);
	if(!saved) {
		g_object_unref(G_OBJECT(file));

		g_warning("failed to save an image from the clipboard: %s",
		          error->message);
		g_error_free(error);

		g_io_stream_close(G_IO_STREAM(stream), NULL, NULL);
		g_object_unref(G_OBJECT(stream));

		return;
	}

	/* Now that all of the data has been written, use g_seekable_tell to get
	 * the size of the file.
	 */
	size = g_seekable_tell(G_SEEKABLE(ostream));

	if(!g_io_stream_close(G_IO_STREAM(stream), NULL, &error)) {
		g_object_unref(G_OBJECT(file));

		g_warning("failed to save an image from the clipboard: %s",
		          error->message);
		g_error_free(error);

		g_object_unref(G_OBJECT(stream));

		return;
	}

	/* now create the message and its attachment */
	input = GTK_WIDGET(data);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));
	comment = talkatu_markup_get_html(buffer, NULL);

	attachment = talkatu_simple_attachment_new(G_GUINT64_CONSTANT(0), "image/png");

	filename = g_file_get_uri(file);
	talkatu_attachment_set_local_uri(attachment, filename);
	g_free(filename);

	/* for the remote side, we'll set the filename to something
	 * non-descriptive.
	 */
	talkatu_attachment_set_remote_uri(attachment, "unknown.png");

	talkatu_attachment_set_size(attachment, size);

	dialog = talkatu_attachment_dialog_new(attachment, comment);
	g_signal_connect(G_OBJECT(dialog), "response",
	                 G_CALLBACK(talkatu_input_attachment_response_cb), input);
	gtk_widget_show_all(dialog);

	g_object_unref(G_OBJECT(file));
	g_free(comment);
	g_object_unref(G_OBJECT(stream));
}

/******************************************************************************
 * Default Signal Handlers
 *****************************************************************************/
static gboolean
talkatu_input_popup_menu(GtkWidget *widget) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(TALKATU_INPUT(widget));
	GtkTextBuffer *buffer = NULL;
	GtkTextIter iter;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));

	gtk_text_buffer_get_iter_at_mark(
		buffer,
		&iter,
		gtk_text_buffer_get_insert(buffer)
	);

	gtk_text_buffer_move_mark(buffer, priv->context_mark, &iter);

	return GTK_WIDGET_CLASS(talkatu_input_parent_class)->popup_menu(widget);
}

static void
talkatu_input_should_send_message(TalkatuInput *input, TalkatuInputSendBinding binding) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	if((priv->send_binding & binding) != 0) {
		talkatu_input_send_message(input);
	} else if(gtk_text_view_get_editable(GTK_TEXT_VIEW(input))) {
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));

		gtk_text_buffer_insert_at_cursor(buffer, "\n", 1);
	}
}

/******************************************************************************
 * GtkTextViewClass overrides
 *****************************************************************************/
static void
talkatu_input_paste_clipboard(GtkTextView *view) {
	GtkClipboard *clipboard =
		gtk_widget_get_clipboard(GTK_WIDGET(view), GDK_SELECTION_CLIPBOARD);

	if(gtk_clipboard_wait_is_image_available(clipboard)) {
		gtk_clipboard_request_image(clipboard, talkatu_input_image_received_cb,
		                            view);
	} else {
		GTK_TEXT_VIEW_CLASS(talkatu_input_parent_class)->paste_clipboard(view);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
talkatu_input_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec) {
	TalkatuInput *input = TALKATU_INPUT(obj);

	switch(prop_id) {
		case PROP_SEND_BINDING:
			g_value_set_flags(value, talkatu_input_get_send_binding(input));
			break;
		case PROP_ID:
			g_value_set_string(value, talkatu_input_get_id(input));
			break;
		case PROP_TIMESTAMP:
			g_value_set_boxed(value, talkatu_input_get_timestamp(input));
			break;
		case PROP_CONTENT_TYPE:
			g_value_set_enum(value, talkatu_input_get_content_type(input));
			break;
		case PROP_AUTHOR:
			g_value_set_string(value, talkatu_input_get_author(input));
			break;
		case PROP_AUTHOR_NAME_COLOR:
			g_value_set_boxed(value, talkatu_input_get_author_name_color(input));
			break;
		case PROP_CONTENTS:
			g_value_set_string(value, talkatu_input_get_contents(input));
			break;
		case PROP_EDITED:
			g_value_set_boolean(value, talkatu_input_get_edited(input));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_input_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec) {
	TalkatuInput *input = TALKATU_INPUT(obj);

	switch(prop_id) {
		case PROP_SEND_BINDING:
			talkatu_input_set_send_binding(input, g_value_get_flags(value));
			break;
		case PROP_ID:
			talkatu_input_set_id(input, g_value_get_string(value));
			break;
		case PROP_TIMESTAMP:
			talkatu_input_set_timestamp(input, g_value_get_boxed(value));
			break;
		case PROP_CONTENT_TYPE:
			talkatu_input_set_content_type(input, g_value_get_enum(value));
			break;
		case PROP_AUTHOR:
			talkatu_input_set_author(input, g_value_get_string(value));
			break;
		case PROP_AUTHOR_NAME_COLOR:
			talkatu_input_set_author_name_color(input, g_value_get_boxed(value));
			break;
		case PROP_CONTENTS:
			talkatu_input_set_contents(input, g_value_get_string(value));
			break;
		case PROP_EDITED:
			talkatu_input_set_edited(input, g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_input_init(TalkatuInput *input) {
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	priv->attachments = g_hash_table_new_full(g_int64_hash, g_int64_equal,
	                                          NULL, g_object_unref);

	/* we need to know when the buffer is changed in our parent so we can
	 * update our actions and other stuff.
	 */
	g_signal_connect(
		G_OBJECT(input),
		"notify::buffer",
		G_CALLBACK(talkatu_input_buffer_set_cb),
		NULL
	);

	/* setup GSpell */
	priv->gspell_view = gspell_text_view_get_from_gtk_text_view(GTK_TEXT_VIEW(input));

	/* we need to connect this signal *AFTER* everything to make sure our items
	 * end up in the correct place.
	 */
	g_signal_connect_after(
		G_OBJECT(input),
		"populate-popup",
		G_CALLBACK(talkatu_input_populate_popup_cb),
		NULL
	);
}

static void
talkatu_input_finalize(GObject *obj) {
	TalkatuInput *input = TALKATU_INPUT(obj);
	TalkatuInputPrivate *priv = talkatu_input_get_instance_private(input);

	g_hash_table_destroy(priv->attachments);

	g_clear_pointer(&priv->timestamp, g_date_time_unref);

	g_clear_pointer(&priv->author, g_free);
	g_clear_pointer(&priv->author_name_color, gdk_rgba_free);

	G_OBJECT_CLASS(talkatu_input_parent_class)->finalize(obj);
}

static void
talkatu_input_class_init(TalkatuInputClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS(klass);
	GtkBindingSet *binding_set = NULL;

	obj_class->get_property = talkatu_input_get_property;
	obj_class->set_property = talkatu_input_set_property;
	obj_class->finalize = talkatu_input_finalize;

	widget_class->popup_menu = talkatu_input_popup_menu;

	text_view_class->paste_clipboard = talkatu_input_paste_clipboard;

	klass->should_send_message = talkatu_input_should_send_message;

	/* add our properties */
	properties[PROP_SEND_BINDING] = g_param_spec_flags(
		"send-binding", "send-binding", "The keybindings that will trigger the send signal",
		TALKATU_TYPE_INPUT_SEND_BINDING,
		TALKATU_INPUT_SEND_BINDING_RETURN | TALKATU_INPUT_SEND_BINDING_KP_ENTER,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT
	);
	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* override the properties for the interface */
	g_object_class_override_property(obj_class, PROP_ID, "id");
	g_object_class_override_property(obj_class, PROP_TIMESTAMP, "timestamp");
	g_object_class_override_property(obj_class, PROP_CONTENT_TYPE, "content-type");
	g_object_class_override_property(obj_class, PROP_AUTHOR, "author");
	g_object_class_override_property(obj_class, PROP_AUTHOR_NAME_COLOR, "author-name-color");
	g_object_class_override_property(obj_class, PROP_CONTENTS, "contents");
	g_object_class_override_property(obj_class, PROP_EDITED, "edited");

	/**
	 * TalkatuInput::should-send-message:
	 * @talkatuinput: The #TalkatuInput instance.
	 * @arg1: The #TalkatuInputSendBinding that was entered.
	 * @user_data: User supplied data.
	 *
	 * Emitted when a potential keybinding to send the message is entered to
	 * determine if the message should be sent.
	 */
	signals[SIG_SHOULD_SEND_MESSAGE] = g_signal_new(
		"should-send-message",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(TalkatuInputClass, should_send_message),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		TALKATU_TYPE_INPUT_SEND_BINDING
	);

	/**
	 * TalkatuInput::send-message:
	 * @talkatuinput: The #TalkatuInput instance.
	 * @user_data: User supplied data.
	 *
	 * Emitted when a message should be sent.
	 */
	signals[SIG_SEND_MESSAGE] = g_signal_new(
		"send-message",
		G_TYPE_FROM_CLASS(klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET(TalkatuInputClass, send_message),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0
	);

	/* setup key bindings */
	binding_set = gtk_binding_set_by_class(talkatu_input_parent_class);

	gtk_binding_entry_add_signal(binding_set, GDK_KEY_Return, 0, "should-send-message", 1, TALKATU_TYPE_INPUT_SEND_BINDING, TALKATU_INPUT_SEND_BINDING_RETURN);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_Return, GDK_SHIFT_MASK, "should-send-message", 1, TALKATU_TYPE_INPUT_SEND_BINDING, TALKATU_INPUT_SEND_BINDING_SHIFT_RETURN);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_Return, GDK_CONTROL_MASK, "should-send-message", 1, TALKATU_TYPE_INPUT_SEND_BINDING, TALKATU_INPUT_SEND_BINDING_CONTROL_RETURN);
	gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_Enter, 0, "should-send-message", 1, TALKATU_TYPE_INPUT_SEND_BINDING, TALKATU_INPUT_SEND_BINDING_KP_ENTER);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_input_new:
 *
 * Creates a new #TalkatuInput instance.
 *
 * Returns: (transfer full): The new #TalkatuInput instance.
 */
GtkWidget *
talkatu_input_new(void) {
	return GTK_WIDGET(g_object_new(TALKATU_TYPE_INPUT, NULL));
}

/**
 * talkatu_input_set_send_binding:
 * @input: The #TalkatuInput instance.
 * @bindings: The #TalkatuInputSendBinding value.
 *
 * Sets the bindings for when the send-message signal should be emitted.
 */
void
talkatu_input_set_send_binding(TalkatuInput *input,
                               TalkatuInputSendBinding bindings)
{
	TalkatuInputPrivate *priv = NULL;

	g_return_if_fail(TALKATU_IS_INPUT(input));

	priv = talkatu_input_get_instance_private(TALKATU_INPUT(input));

	priv->send_binding = bindings;

	g_object_notify_by_pspec(G_OBJECT(input), properties[PROP_SEND_BINDING]);
}

/**
 * talkatu_input_get_send_binding:
 * @input: The #TalkatuInput instance.
 *
 * Gets the #TalkatuInputSendBinding which determines when send-message
 * signal will be emitted.
 *
 * Returns: The #TalkatuInputSendBinding.
 */
TalkatuInputSendBinding
talkatu_input_get_send_binding(TalkatuInput *input) {
	TalkatuInputPrivate *priv = NULL;

	g_return_val_if_fail(TALKATU_IS_INPUT(input), 0);

	priv = talkatu_input_get_instance_private(TALKATU_INPUT(priv));

	return priv->send_binding;
}

/**
 * talkatu_input_send_message:
 * @input: The #TalkatuInput instance.
 *
 * Emits the signal that @input is trying to send a message.  This is used for
 * cases like the optional send button in #TalkatuEditor and other instances
 * where the user has performed an action to send a message.
 */
void
talkatu_input_send_message(TalkatuInput *input) {
	g_return_if_fail(TALKATU_IS_INPUT(input));

	g_signal_emit(input, signals[SIG_SEND_MESSAGE], 0);
}
