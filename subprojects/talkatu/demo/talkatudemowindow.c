/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
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
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "talkatudemowindow.h"

struct _TalkatuDemoWindow {
	GtkWindow parent;

	GtkWidget *history;
	GtkWidget *editor;
	GtkWidget *typing;

	GtkRadioToolButton *toggle_plain;
	GtkRadioToolButton *toggle_whole;
	GtkRadioToolButton *toggle_html;
	GtkRadioToolButton *toggle_markdown;

	GtkToggleToolButton *toggle_toolbar;
	GtkToggleToolButton *toggle_send_button;
	GtkToggleToolButton *toggle_edited;

	GtkTextBuffer *buffer_plain;
	GtkTextBuffer *buffer_whole;
	GtkTextBuffer *buffer_html;
	GtkTextBuffer *buffer_markdown;

	GtkWidget *author_button;
	GtkWidget *author_popover;
	GtkWidget *author_item;
};

G_DEFINE_TYPE(TalkatuDemoWindow, talkatu_demo_window, GTK_TYPE_WINDOW);

static void
talkatu_demo_window_insert_html_cb(GtkButton *toggle, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkWidget *dialog = NULL;
	GtkFileFilter *filter = NULL;
	gint res;

	dialog = gtk_file_chooser_dialog_new(
		_("insert html..."),
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"OK", GTK_RESPONSE_ACCEPT,
		"Cancel", GTK_RESPONSE_REJECT,
		NULL
	);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "HTML files");
	gtk_file_filter_add_pattern(filter, "*.html");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gchar *contents = NULL;
		gsize len;

		if(g_file_get_contents(filename, &contents, &len, NULL)) {
			GtkTextMark *mark = NULL;
			GtkTextIter iter;

			mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(window->buffer_html));
			gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(window->buffer_html), &iter, mark);

			talkatu_html_buffer_insert_html(TALKATU_HTML_BUFFER(window->buffer_html), &iter, contents, len);
			g_free(contents);
		}

		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void
talkatu_demo_window_insert_markdown_cb(GtkButton *toggle, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkWidget *dialog = NULL;
	GtkFileFilter *filter = NULL;
	gint res;

	dialog = gtk_file_chooser_dialog_new(
		_("insert markdown..."),
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"OK", GTK_RESPONSE_ACCEPT,
		"Cancel", GTK_RESPONSE_REJECT,
		NULL
	);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Markdown files");
	gtk_file_filter_add_pattern(filter, "*.md");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gchar *contents = NULL;
		gsize len;

		if(g_file_get_contents(filename, &contents, &len, NULL)) {
			GtkTextMark *mark = NULL;
			GtkTextIter iter;

			mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(window->buffer_markdown));
			gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(window->buffer_markdown), &iter, mark);

			talkatu_markdown_buffer_insert_markdown(TALKATU_MARKDOWN_BUFFER(window->buffer_markdown), &iter, contents, len);
			g_free(contents);
		}

		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void
talkatu_demo_window_buffer_changed_cb(GtkToggleButton *toggle, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkWidget *view = talkatu_editor_get_input(TALKATU_EDITOR(window->editor));

	if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(window->toggle_plain))) {
		g_message("switching to plain buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_plain);
	} else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(window->toggle_whole))) {
		g_message("switching to whole buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_whole);
	} else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(window->toggle_html))) {
		g_message("switching to html buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_html);
	} else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(window->toggle_markdown))) {
		g_message("switching to markdown buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_markdown);
	}
}

static void
talkatu_demo_window_buffer_modified_cb(GtkTextBuffer *buffer, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkWidget *input = talkatu_editor_get_input(TALKATU_EDITOR(window->editor));
	gchar *author = NULL;

	author = talkatu_message_get_author(TALKATU_MESSAGE(input));

	if(gtk_text_buffer_get_char_count(buffer) > 0) {
		talkatu_typing_label_start_typing(TALKATU_TYPING_LABEL(window->typing),
		                                  author);
	} else {
		talkatu_typing_label_finish_typing(TALKATU_TYPING_LABEL(window->typing),
		                                   author);
	}

	g_free(author);
}

static void
talkatu_demo_window_view_open_url_cb(TalkatuView *view, const gchar *url, gpointer data) {
	GError *error = NULL;
	gboolean success = FALSE;

	success = gtk_show_uri_on_window(
	        GTK_WINDOW(data), url, GDK_CURRENT_TIME, &error);
	if(!success) {
		g_message(
			"failed to open uri '%s': %s",
			url,
			(error) ? error->message : NULL
		);

		g_error_free(error);
	}
}

static void
talkatu_demo_window_view_send_message_cb(TalkatuInput *input, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	gchar *sid = NULL;
	static guint64 id = 0;

	sid = g_strdup_printf("%" G_GUINT64_FORMAT, id++);
	talkatu_message_set_id(TALKATU_MESSAGE(input), sid);
	g_free(sid);

	talkatu_history_write_message(
		TALKATU_HISTORY(window->history),
		TALKATU_MESSAGE(input)
	);

	talkatu_message_set_contents(TALKATU_MESSAGE(input), "");
	talkatu_message_clear_attachments(TALKATU_MESSAGE(input));
}

static gboolean
talkatu_demo_window_closed_cb(GtkWidget *w, GdkEvent *e, gpointer d) {
	gtk_main_quit();

	return FALSE;
}

static void
talkatu_demo_window_author_toggled_cb(GtkToolButton *button, gpointer data) {
	GtkPopover *popover = GTK_POPOVER(data);

	gtk_popover_popup(popover);
}

static void
talkatu_demo_window_author_popover_closed_cb(GtkPopover *popover, gpointer data) {
	GtkToggleToolButton *button = GTK_TOGGLE_TOOL_BUTTON(data);

	gtk_toggle_tool_button_set_active(button, FALSE);
}

static void
talkatu_demo_window_author_changed(GtkRadioButton *item, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item))) {
		TalkatuEditor *editor = TALKATU_EDITOR(window->editor);
		GtkWidget *input = talkatu_editor_get_input(editor);

		talkatu_message_set_author(TALKATU_MESSAGE(input),
		                           gtk_button_get_label(GTK_BUTTON(item)));
	}
}

static void
talkatu_demo_window_author_name_color_toggled_cb(GtkToolButton *button,
                                                 gpointer data)
{
	GtkPopover *popover = GTK_POPOVER(data);

	gtk_popover_popup(popover);
}

static void
talkatu_demo_window_author_name_color_popover_closed_cb(GtkPopover *popover,
                                                        gpointer data)
{
	GtkToggleToolButton *button = GTK_TOGGLE_TOOL_BUTTON(data);

	gtk_toggle_tool_button_set_active(button, FALSE);
}

static void
talkatu_demo_window_author_name_color_changed(GtkRadioButton *item,
                                              gpointer data)
{
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item))) {
		TalkatuEditor *editor = TALKATU_EDITOR(window->editor);
		GtkWidget *input = talkatu_editor_get_input(editor);
		GdkRGBA color;
		const gchar *label = gtk_button_get_label(GTK_BUTTON(item));

		if(gdk_rgba_parse(&color, label)) {
			talkatu_message_set_author_name_color(TALKATU_MESSAGE(input),
			                                      &color);
		} else {
			talkatu_message_set_author_name_color(TALKATU_MESSAGE(input), NULL);
		}
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
talkatu_demo_window_init(TalkatuDemoWindow *window) {
	gtk_widget_init_template(GTK_WIDGET(window));

	/* activate the first menu item to make sure its label gets stored
	 * correctly.
	 */
	talkatu_demo_window_author_changed(GTK_RADIO_BUTTON(window->author_item), window);

	g_object_bind_property(
		window->editor, "show-toolbar",
		window->toggle_toolbar, "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL
	);

	g_object_bind_property(
		window->editor, "show-send-button",
		window->toggle_send_button, "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL
	);

	g_object_bind_property(
		talkatu_editor_get_input(TALKATU_EDITOR(window->editor)), "edited",
		window->toggle_edited, "active",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL
	);
}

static void
talkatu_demo_window_class_init(TalkatuDemoWindowClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/demo/demo.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, history);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, editor);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, typing);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_plain);
 	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_whole);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_html);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_markdown);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_plain);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_whole);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_html);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_markdown);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, author_button);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, author_popover);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, author_item);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_toolbar);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_send_button);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_edited);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_closed_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_buffer_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_buffer_modified_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_view_open_url_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_view_send_message_cb);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_insert_html_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_insert_markdown_cb);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_author_toggled_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_author_popover_closed_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_author_changed);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_author_name_color_toggled_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_author_name_color_popover_closed_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_author_name_color_changed);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
talkatu_demo_window_new(void) {
	return GTK_WIDGET(g_object_new(TALKATU_DEMO_TYPE_WINDOW, NULL));
}
