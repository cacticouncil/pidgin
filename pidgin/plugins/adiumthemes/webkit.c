/*
 * Adium Message Styles
 * Copyright (C) 2009  Arnold Noronha <arnstein87@gmail.com>
 * Copyright (C) 2007
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#define PLUGIN_ID		"gtk-webview-adium-ims"
#define PLUGIN_NAME		"webview-adium-ims"

/*
 * A lot of this was originally written by Sean Egan, but I think I've 
 * rewrote enough to replace the author for now. 
 */
#define PLUGIN_AUTHOR		"Arnold Noronha <arnstein87@gmail.com>" 
#define PURPLE_PLUGINS          "Hell yeah"

/* System headers */
#include <string.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <webkit/webkit.h>

/* Purple headers */
#include <conversation.h>
#include <notify.h>
#include <util.h>
#include <version.h>

/* Pidgin headers */
#include <gtkconv.h>
#include <gtkplugin.h>
#include <gtkwebview.h>
#include <smileyparser.h>

/*
 * I'm going to allow a different style for each PidginConversation.
 * This way I can do two things: 1) change the theme on the fly and not
 * change existing themes, and 2) Use a different theme for IMs and
 * chats.
 */
typedef struct _PidginMessageStyle {
	int     ref_counter;

	/* paths */
	char    *style_dir;
	char    *template_path;
	char    *css_path;

	/* caches */
	char    *template_html;
	char    *header_html;
	char    *footer_html;
	char    *incoming_content_html;
	char    *outgoing_content_html;
	char    *incoming_next_content_html;
	char    *outgoing_next_content_html;
	char    *status_html;
	char    *basestyle_css;
} PidginMessageStyle;

static GList *style_list; /**< List of PidginMessageStyles */
static char  *cur_style_dir = NULL;
static void  *handle = NULL;

static PidginMessageStyle* pidgin_message_style_new ()
{
	PidginMessageStyle* ret = g_new0 (PidginMessageStyle, 1);
	ret->ref_counter ++;
	
	style_list = g_list_append (style_list, ret);
	return ret;
}

static void pidgin_message_style_unref (PidginMessageStyle *style)
{
	style->ref_counter--;
	if (style->ref_counter) return;

	g_free (style->style_dir);
	g_free (style->template_path);
	g_free (style->css_path);
	
	g_free (style->template_html);
	g_free (style->incoming_content_html);
	g_free (style->outgoing_content_html);
	g_free (style->outgoing_next_content_html);
	g_free (style->status_html);
	g_free (style->basestyle_css);

	style_list = g_list_remove (style_list, style);
	g_free (style);
}

static void variant_set_default (PidginMessageStyle* style);
static PidginMessageStyle*
pidgin_message_style_load (const char* styledir)
{
	/* is this style already loaded? */
	GList  *cur = style_list;
	char   *file; /* temporary variable */
	PidginMessageStyle *style = NULL;
	
	g_assert (styledir);

	for (; cur; cur = g_list_next (cur)) {
		if (g_str_equal (styledir, ((PidginMessageStyle*) cur->data)->style_dir))
			return (PidginMessageStyle*) cur->data;
	}

	/* else we need to load it */
	style = pidgin_message_style_new ();
	style->style_dir = g_strdup (styledir);
	
	/* find some variant file (or load from user's settings) */
	variant_set_default (style);

	/* load all other files */

	/* The template path can either come from the theme, or can
	 * be stock Template.html that comes with the plugin */
	style->template_path = g_build_filename(styledir, "Contents", "Resources", "Template.html", NULL);

	if (!g_file_test(style->template_path, G_FILE_TEST_EXISTS)) {
		g_free (style->template_path);
		style->template_path = g_build_filename(DATADIR, "pidgin", "webkit", "Template.html", NULL);
	}

	if (!g_file_get_contents(style->template_path, &style->template_html, NULL, NULL)) {
		pidgin_message_style_unref (style);
		return NULL;
	}

	file = g_build_filename(styledir, "Contents", "Resources", "Status.html", NULL);
	if (!g_file_get_contents(file, &style->status_html, NULL, NULL)) {
		pidgin_message_style_unref (style);
		g_free (file);
		return NULL;
	}
	g_free (file);

	file = g_build_filename(styledir, "Contents", "Resources", "main.css", NULL);
	g_file_get_contents(file, &style->basestyle_css, NULL, NULL);
	g_free (file);
	
	file = g_build_filename(styledir, "Contents", "Resources", "Header.html", NULL);
	g_file_get_contents(file, &style->header_html, NULL, NULL);
	g_free (file);

	file = g_build_filename(styledir, "Contents", "Resources", "Footer.html", NULL);
	g_file_get_contents(file, &style->footer_html, NULL, NULL);
	g_free (file);



	file = g_build_filename(styledir, "Contents", "Resources", "Incoming", "Content.html", NULL);
	if (!g_file_get_contents(file, &style->incoming_content_html, NULL, NULL)) {
		pidgin_message_style_unref (style);
		g_free (file);
		return NULL;
	}
	g_free (file);


	/* according to the spec, the following are not necessary files */
	file = g_build_filename(styledir, "Contents", "Resources", "Incoming", "NextContent.html", NULL);
	if (!g_file_get_contents(file, &style->incoming_next_content_html, NULL, NULL)) {
		style->incoming_next_content_html = g_strdup (style->incoming_content_html);
	}
	g_free (file);

	file = g_build_filename(styledir, "Contents", "Resources", "Outgoing", "Content.html", NULL);
	if (!g_file_get_contents(file, &style->outgoing_content_html, NULL, NULL)) {
		style->outgoing_content_html = g_strdup(style->incoming_content_html);
	}
	g_free (file);

	file = g_build_filename(styledir, "Contents", "Resources", "Outgoing", "NextContent.html", NULL);
	if (!g_file_get_contents(file, &style->outgoing_next_content_html, NULL, NULL)) {
		style->outgoing_next_content_html = g_strdup (style->outgoing_content_html);
	}

	return style;
}


static void* webkit_plugin_get_handle ()
{
	if (handle) return handle;
	else return (handle = g_malloc (1));
}

static void webkit_plugin_free_handle ()
{
	purple_signals_disconnect_by_handle (handle);
	g_free (handle);
}

static char *
replace_message_tokens(
	char *text, 
	gsize len, 
	PurpleConversation *conv, 
	const char *name, 
	const char *alias, 
	const char *message, 
	PurpleMessageFlags flags, 
	time_t mtime)
{
	GString *str = g_string_new_len(NULL, len);
	char *cur = text;
	char *prev = cur;

	while ((cur = strchr(cur, '%'))) {
		const char *replace = NULL;
		char *fin = NULL;
			
		if (!strncmp(cur, "%message%", strlen("%message%"))) {
			replace = message;
		} else if (!strncmp(cur, "%messageClasses%", strlen("%messageClasses%"))) {
			replace = flags & PURPLE_MESSAGE_SEND ? "outgoing" :
				  flags & PURPLE_MESSAGE_RECV ? "incoming" : "event";
		} else if (!strncmp(cur, "%time", strlen("%time"))) {
			char *format = NULL;
			if (*(cur + strlen("%time")) == '{') {
				char *start = cur + strlen("%time") + 1;
				char *end = strstr(start, "}%");
				if (!end) /* Invalid string */
					continue;
				format = g_strndup(start, end - start);
				fin = end + 1;
			} 
			replace = purple_utf8_strftime(format ? format : "%X", NULL);
			g_free(format);
		} else if (!strncmp(cur, "%userIconPath%", strlen("%userIconPath%"))) {
			if (flags & PURPLE_MESSAGE_SEND) {
				if (purple_account_get_bool(conv->account, "use-global-buddyicon", TRUE)) {
					replace = purple_prefs_get_path(PIDGIN_PREFS_ROOT "/accounts/buddyicon");
				} else {
					PurpleStoredImage *img = purple_buddy_icons_find_account_icon(conv->account);
					replace = purple_imgstore_get_filename(img);
				}
				if (replace == NULL || !g_file_test(replace, G_FILE_TEST_EXISTS)) {
					replace = g_build_filename("Outgoing", "buddy_icon.png", NULL);
				}
			} else if (flags & PURPLE_MESSAGE_RECV) {
				PurpleBuddyIcon *icon = purple_conv_im_get_icon(PURPLE_CONV_IM(conv));
				replace = purple_buddy_icon_get_full_path(icon);
				if (replace == NULL || !g_file_test(replace, G_FILE_TEST_EXISTS)) {
					replace = g_build_filename("Incoming", "buddy_icon.png", NULL);
				}
			}
			
		} else if (!strncmp(cur, "%senderScreenName%", strlen("%senderScreenName%"))) {
			replace = name;
		} else if (!strncmp(cur, "%sender%", strlen("%sender%"))) {
			replace = alias;
		} else if (!strncmp(cur, "%service%", strlen("%service%"))) {
			replace = purple_account_get_protocol_name(conv->account);
		} else {
			cur++;
			continue;
		}

		/* Here we have a replacement to make */
		g_string_append_len(str, prev, cur - prev);
		g_string_append(str, replace);

		/* And update the pointers */
		if (fin) {
			prev = cur = fin + 1;	
		} else {
			prev = cur = strchr(cur + 1, '%') + 1;
		}

	}
	
	/* And wrap it up */
	g_string_append(str, prev);
	return g_string_free(str, FALSE);
}

static char *
replace_header_tokens(char *text, gsize len, PurpleConversation *conv)
{
	GString *str = g_string_new_len(NULL, len);
	char *cur = text;
	char *prev = cur;

	if (text == NULL)
		return NULL;

	while ((cur = strchr(cur, '%'))) {
		const char *replace = NULL;
		char *fin = NULL;

  		if (!strncmp(cur, "%chatName%", strlen("%chatName%"))) {
			replace = conv->name;
  		} else if (!strncmp(cur, "%sourceName%", strlen("%sourceName%"))) {
			replace = purple_account_get_alias(conv->account);
			if (replace == NULL)
				replace = purple_account_get_username(conv->account);
  		} else if (!strncmp(cur, "%destinationName%", strlen("%destinationName%"))) {
			PurpleBuddy *buddy = purple_find_buddy(conv->account, conv->name);
			if (buddy) {
				replace = purple_buddy_get_alias(buddy);
			} else {
				replace = conv->name;
			}
  		} else if (!strncmp(cur, "%incomingIconPath%", strlen("%incomingIconPath%"))) {
			PurpleBuddyIcon *icon = purple_conv_im_get_icon(PURPLE_CONV_IM(conv));
			replace = purple_buddy_icon_get_full_path(icon);
  		} else if (!strncmp(cur, "%outgoingIconPath%", strlen("%outgoingIconPath%"))) {
  		} else if (!strncmp(cur, "%timeOpened", strlen("%timeOpened"))) {
			char *format = NULL;
			if (*(cur + strlen("%timeOpened")) == '{') {
				char *start = cur + strlen("%timeOpened") + 1;
				char *end = strstr(start, "}%");
				if (!end) /* Invalid string */
					continue;
				format = g_strndup(start, end - start);
				fin = end + 1;
			} 
			replace = purple_utf8_strftime(format ? format : "%X", NULL);
			g_free(format);
		} else {
			continue;
		}

		/* Here we have a replacement to make */
		g_string_append_len(str, prev, cur - prev);
		g_string_append(str, replace);

		/* And update the pointers */
		if (fin) {
			prev = cur = fin + 1;	
		} else {
			prev = cur = strchr(cur + 1, '%') + 1;
		}
	}
	
	/* And wrap it up */
	g_string_append(str, prev);
	return g_string_free(str, FALSE);
}

static char *
replace_template_tokens(PidginMessageStyle *style, char *text, int len, char *header, char *footer) {
	GString *str = g_string_new_len(NULL, len);

	char **ms = g_strsplit(text, "%@", 6);
	char *base = NULL;

	if (ms[0] == NULL || ms[1] == NULL || ms[2] == NULL || ms[3] == NULL || ms[4] == NULL || ms[5] == NULL) {
		g_strfreev(ms);
		g_string_free(str, TRUE);
		return NULL;
	}

	g_string_append(str, ms[0]);
	g_string_append(str, "file://");
	base = g_build_filename (style->style_dir, "Contents", "Resources", "Template.html", NULL);
	g_string_append(str, base);
	g_free (base);

	g_string_append(str, ms[1]);
	if (style->basestyle_css)
		g_string_append(str, style->basestyle_css);
	g_string_append(str, ms[2]);
	if (style->css_path) {
		g_string_append(str, "file://");
		g_string_append(str, style->css_path);
	}

	g_string_append(str, ms[3]);
	if (header)
		g_string_append(str, header);
	g_string_append(str, ms[4]);
	if (footer)
		g_string_append(str, footer);
	g_string_append(str, ms[5]);
	
	g_strfreev(ms);
	return g_string_free (str, FALSE);
}

static GtkWidget *
get_webkit(PurpleConversation *conv)
{
	PidginConversation *gtkconv;
	gtkconv = PIDGIN_CONVERSATION(conv);
	if (!gtkconv)
		return NULL;
	else 
		return gtkconv->webview;
}

/**
 * Called when either a new PurpleConversation is created
 * or when a PidginConversation changes its active PurpleConversation
 *
 * FIXME: it's not at all clear to me as to how
 * Adium themes handle the case when the PurpleConversation
 * changes.
 */
static void
init_theme_for_webkit (PurpleConversation *conv, char *style_dir)
{
	GtkWidget *webkit = PIDGIN_CONVERSATION(conv)->webview;
	char *header, *footer;
	char *template;

	char* basedir;
	char* baseuri;
	PidginMessageStyle *style;
	style = pidgin_message_style_load (style_dir);
	g_assert (style);

	basedir = g_build_filename (style->style_dir, "Contents", "Resources", "Template.html", NULL);
	baseuri = g_strdup_printf ("file://%s", basedir);
	header = replace_header_tokens(style->header_html, strlen(style->header_html), conv);
	footer = replace_header_tokens(style->footer_html, strlen(style->footer_html), conv);
	template = replace_template_tokens(style, style->template_html, strlen(style->template_html) + strlen(style->header_html), header, footer);

	if (!g_object_get_data (G_OBJECT(webkit), "adium-themed"))
		webkit_web_view_load_string(WEBKIT_WEB_VIEW(webkit), template, "text/html", "UTF-8", baseuri);

	g_object_set_data (G_OBJECT(webkit), "adium-themed", style);

	g_free (basedir);
	g_free (baseuri);
	g_free (header);
	g_free (footer);
	g_free (template);
}


/* restore the non theme version of the conversation window */
static void
finalize_theme_for_webkit (PurpleConversation *conv)
{
	GtkWidget *webview = PIDGIN_CONVERSATION(conv)->webview;
	PidginMessageStyle *style = g_object_get_data (G_OBJECT(webview), "adium-themed");

	webkit_web_view_load_string(WEBKIT_WEB_VIEW(webview), "", "text/html", "UTF-8", "");
	g_object_set_data (G_OBJECT(webview), "adium-themed", NULL);
	pidgin_message_style_unref (style);
}

struct webkit_script {
	GtkWidget *webkit;
	char *script;
};

static gboolean purple_webkit_execute_script(gpointer _script)
{
	struct webkit_script *script = (struct webkit_script*) _script;
	printf ("%s\n", script->script);
	webkit_web_view_execute_script(WEBKIT_WEB_VIEW(script->webkit), script->script);
	g_free(script->script);
	g_free(script);
	return FALSE;
}


static gboolean webkit_on_displaying_im_msg (PurpleAccount *account,
						 const char* name,
						 char **pmessage,
						 PurpleConversation *conv,
						 PurpleMessageFlags flags,
						 gpointer data)
{
	GtkWidget *webkit;
	char *message = *pmessage;
	const char *alias = name; /* FIXME: signal doesn't give me alias */
	char *stripped;
	char *message_html;
	char *msg;
	char *escape;
	char *script;
	char *func = "appendMessage";
	char *smileyed;
	time_t mtime = time (NULL); /* FIXME: this should come from the write_conv calback, but the signal doesn't pass this to me */

	struct webkit_script *wk_script;
	PurpleMessageFlags old_flags = GPOINTER_TO_INT(purple_conversation_get_data(conv, "webkit-lastflags")); 
	PidginMessageStyle *style;

	fprintf (stderr, "hmm.. here %s %s\n", name, message);
	webkit = get_webkit(conv);
	stripped = g_strdup(message);

	style = g_object_get_data (G_OBJECT (webkit), "adium-themed");

	if (flags & PURPLE_MESSAGE_SEND && old_flags & PURPLE_MESSAGE_SEND) {
		message_html = style->outgoing_next_content_html;
		func = "appendNextMessage";
	} else if (flags & PURPLE_MESSAGE_SEND) {
		message_html = style->outgoing_content_html;
	} else if (flags & PURPLE_MESSAGE_RECV && old_flags & PURPLE_MESSAGE_RECV) {
		message_html = style->incoming_next_content_html;
		func = "appendNextMessage";
	} else if (flags & PURPLE_MESSAGE_RECV) {
		message_html = style->incoming_content_html;
	} else {
		message_html = style->status_html;
	}
	purple_conversation_set_data(conv, "webkit-lastflags", GINT_TO_POINTER(flags));

	smileyed = smiley_parse_markup(stripped, conv->account->protocol_id);
	msg = replace_message_tokens(message_html, 0, conv, name, alias, smileyed, flags, mtime);
	escape = gtk_webview_quote_js_string (msg);
	script = g_strdup_printf("%s(%s)", func, escape);

	wk_script = g_new0(struct webkit_script, 1);
	wk_script->script = script;
	wk_script->webkit = webkit;

	g_idle_add(purple_webkit_execute_script, wk_script);

	g_free(smileyed);
	g_free(msg);
	g_free(stripped);
	g_free(escape);

	return TRUE; /* GtkConv should not handle this guy */
}

static gboolean webkit_on_displaying_chat_msg (PurpleAccount *account,
					       const char *who,
					       char **message,
					       PurpleConversation *conv,
					       PurpleMessageFlags flags,
					       gpointer userdata)
{
	/* handle exactly like an IM message */
	return webkit_on_displaying_im_msg (account, who, message, conv, flags, NULL);
}

static void
webkit_on_converstation_displayed (PidginConversation *gtkconv, gpointer data)
{
	init_theme_for_webkit (gtkconv->active_conv, cur_style_dir);
}

static void
webkit_on_conversation_switched (PurpleConversation *conv, gpointer data)
{
	init_theme_for_webkit (conv, cur_style_dir);
}

static void
webkit_on_conversation_hiding (PidginConversation *gtkconv, gpointer data)
{
	/* 
	 * I'm not sure if I need to do anything here, but let's keep
	 * this anyway
	 */
}

/**
 * Get each of the files corresponding to each variant.
 */
static GList*
get_theme_files(PidginMessageStyle *style) 
{
	GList *ret = NULL;
        GDir *variants;
	const char *css_file;
	char *css;
	char *variant_dir;

	g_assert (style->style_dir);
	variant_dir = g_build_filename(style->style_dir, "Contents", "Resources", "Variants", NULL);

	variants = g_dir_open(variant_dir, 0, NULL);
	if (!variants) return NULL;

	while ((css_file = g_dir_read_name(variants)) != NULL) {
		if (!g_str_has_suffix (css_file, ".css"))
			continue;

		css = g_build_filename(variant_dir, css_file, NULL);
		ret = g_list_append(ret, css);
	}

	g_dir_close(variants);
	g_free(variant_dir);

	ret = g_list_sort (ret, (GCompareFunc)g_strcmp0);
	return ret;	
}

static void
variant_set_default (PidginMessageStyle* style)
{
	GList *all, *iter;
	const char *css_path = purple_prefs_get_string ("/plugins/gtk/adiumthemes/csspath");

	g_free (style->css_path);
	if (g_str_has_prefix (css_path, style->style_dir) &&
	    g_file_test (css_path, G_FILE_TEST_EXISTS)) {
		style->css_path = g_strdup (css_path);
		return;
	}
	else {
		/* something about the theme has changed */
		css_path = NULL;
	}
	
	all = get_theme_files (style);

	if (all) {
		style->css_path = g_strdup (all->data);
		purple_prefs_set_string ("/plugins/gtk/adiumthemes/csspath", css_path);
	}
	
	for (iter = all; iter; iter = g_list_next (iter))
		g_free (iter->data);

	g_list_free (all);
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
	PidginMessageStyle *_style; /* temp */

	if (g_path_is_absolute (purple_user_dir())) 
		cur_style_dir = g_build_filename(purple_user_dir(), "style", NULL);
	else {
		char* cur = g_get_current_dir ();
		cur_style_dir = g_build_filename (cur, purple_user_dir(), "style", NULL);
		g_free (cur);
	}
	

	_style = pidgin_message_style_load (cur_style_dir);
	if (!_style) {
		g_free (cur_style_dir);
		cur_style_dir = NULL;
		return FALSE;
	}

	pidgin_message_style_unref (_style);

	purple_signal_connect (pidgin_conversations_get_handle (),
			       "displaying-im-msg",
			       webkit_plugin_get_handle (),
			       PURPLE_CALLBACK(webkit_on_displaying_im_msg),
			       NULL);
			    
	purple_signal_connect (pidgin_conversations_get_handle (),
			       "displaying-chat-msg",
			       webkit_plugin_get_handle (),
			       PURPLE_CALLBACK(webkit_on_displaying_chat_msg),
			       NULL);

	purple_signal_connect (pidgin_conversations_get_handle (),
			       "conversation-displayed",
			       webkit_plugin_get_handle (),
			       PURPLE_CALLBACK(webkit_on_converstation_displayed),
			       NULL);

	purple_signal_connect (pidgin_conversations_get_handle (),
			       "conversation-switched",
			       webkit_plugin_get_handle (),
			       PURPLE_CALLBACK(webkit_on_conversation_switched),
			       NULL);

	purple_signal_connect (pidgin_conversations_get_handle (),
			       "conversation-hiding",
			       webkit_plugin_get_handle (),
			       PURPLE_CALLBACK(webkit_on_conversation_hiding),
			       NULL);

	/* finally update each of the existing conversation windows */
	{
		GList* list = purple_get_conversations ();
		for (;list; list = g_list_next(list))
			init_theme_for_webkit (list->data, cur_style_dir);
			
	}
	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	GList *list;

	webkit_plugin_free_handle ();

	list = purple_get_conversations ();
	while (list) {
		finalize_theme_for_webkit(list->data);
		list = g_list_next(list);
	}

	return TRUE;
}


static void
variant_update_conversation (PurpleConversation *conv)
{
	PidginConversation *gtkconv = PIDGIN_CONVERSATION (conv);
	WebKitWebView *webview = WEBKIT_WEB_VIEW (gtkconv->webview);
	PidginMessageStyle *style = (PidginMessageStyle*) g_object_get_data (G_OBJECT(webview), "adium-themed");

	char* script = g_strdup_printf ("setStylesheet(\"mainStyle\",\"%s\")", style->css_path);
	webkit_web_view_execute_script (webview, script);
	g_free (script);
}

static void
variant_changed (GtkWidget* combobox, gpointer null)
{
	char *name, *name_with_ext;
	char *css_path;
	GList *list;
	PidginMessageStyle *style = pidgin_message_style_load (cur_style_dir);

	g_assert (style);

	/* it is possible that the theme changed by this point, so we check
	 * that first */

	name = gtk_combo_box_get_active_text (GTK_COMBO_BOX(combobox));
	name_with_ext = g_strdup_printf ("%s.css", name);
	g_free (name);
	
	css_path = g_build_filename (style->style_dir, "Contents", "Resources", "Variants", name_with_ext, NULL);

	if (!g_file_test (css_path, G_FILE_TEST_EXISTS)) {
		goto cleanup;
	}

	g_free (style->css_path);
	style->css_path = g_strdup (css_path);
	purple_prefs_set_string ("/plugins/gtk/adiumthemes/csspath", css_path);

	/* update each conversation */
	list = purple_get_conversations ();
	while (list) {
		variant_update_conversation (list->data);
		list = g_list_next(list);
	}

cleanup:
	g_free (css_path);
	g_free (name_with_ext);
	pidgin_message_style_unref (style);
}

static GtkWidget *
get_config_frame(PurplePlugin *plugin) 
{
	PidginMessageStyle *style = pidgin_message_style_load (cur_style_dir);
	GList *themes = get_theme_files(style);
	GList *theme = themes;
	char *curdir = NULL;
	GtkWidget *combobox = gtk_combo_box_new_text();	
	int def = -1, index = 0;
	char* css_path = style->css_path;

	pidgin_message_style_unref (style);

	while (theme) {
		char *basename = g_path_get_basename(theme->data);
		char *dirname = g_path_get_dirname(theme->data);
		if (!curdir || strcmp(curdir, dirname)) {
			char *plist, *plist_xml;
			gsize plist_len;
			xmlnode *node;
			g_free(curdir);
			curdir = strdup(dirname);
			plist = g_build_filename(curdir, "..", "..", "Info.plist", NULL);
		        if (!g_file_get_contents(plist, &plist_xml, &plist_len, NULL)) {
				continue;
			}
	                node = xmlnode_from_str(plist_xml, plist_len);
			if (!node) continue;
			node = xmlnode_get_child(node, "dict");
			if (!node) continue;
			node = xmlnode_get_child(node, "key");
			while (node && strcmp(xmlnode_get_data(node), "CFBundleName")) {
				node = xmlnode_get_next_twin(node);
			}
			if (!node) continue;
			node = node->next;
			while (node && node->type != XMLNODE_TYPE_TAG) {
				node = node->next;
			}

		}
		
		{
			char *temp = g_strndup (basename, strlen(basename)-4);
			gtk_combo_box_append_text (GTK_COMBO_BOX(combobox), temp);
			g_free (temp);
		}
		if (g_str_has_suffix (css_path, basename))
			def = index;
		index ++;
		theme = theme->next;
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX(combobox), def);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(variant_changed), NULL);

	return combobox;
}

PidginPluginUiInfo ui_info =
{
        get_config_frame,
        0, /* page_num (Reserved) */

        /* padding */
        NULL,
        NULL,
        NULL,
        NULL
};


static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,		/* Magic				*/
	PURPLE_MAJOR_VERSION,		/* Purple Major Version	*/
	PURPLE_MINOR_VERSION,		/* Purple Minor Version	*/
	PURPLE_PLUGIN_STANDARD,		/* plugin type			*/
PIDGIN_PLUGIN_TYPE,			/* ui requirement		*/
	0,							/* flags				*/
	NULL,						/* dependencies			*/
	PURPLE_PRIORITY_DEFAULT,	/* priority				*/

	PLUGIN_ID,					/* plugin id			*/
	NULL,						/* name					*/
	"0.1",					/* version				*/
	NULL,						/* summary				*/
	NULL,						/* description			*/
	PLUGIN_AUTHOR,				/* author				*/
	"http://pidgin.im",					/* website				*/

	plugin_load,				/* load					*/
	plugin_unload,				/* unload				*/
	NULL,						/* destroy				*/

	&ui_info,						/* ui_info				*/
	NULL,						/* extra_info			*/
	NULL,						/* prefs_info			*/
	NULL,						/* actions				*/
	NULL,						/* reserved 1			*/
	NULL,						/* reserved 2			*/
	NULL,						/* reserved 3			*/
	NULL						/* reserved 4			*/
};

static void
init_plugin(PurplePlugin *plugin) {
	info.name = "Adium IMs";
	info.summary = "Adium-like IMs with Pidgin";
	info.description = "You can chat in Pidgin using Adium's WebKit view.";

	purple_prefs_add_none ("/plugins");
	purple_prefs_add_none ("/plugins/gtk");
	purple_prefs_add_none ("/plugins/gtk/adiumthemes");
	purple_prefs_add_string ("/plugins/gtk/adiumthemes/csspath", "");
}

PURPLE_INIT_PLUGIN(webkit, init_plugin, info)
