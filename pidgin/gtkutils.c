/* pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <purple.h>

#ifdef _WIN32
#  undef small
#  include <shellapi.h>
#endif /*_WIN32*/

#include <gdk/gdkkeysyms.h>

#include <talkatu.h>

#include "gtkaccount.h"

#include "gtkconv.h"
#include "gtkdialogs.h"
#include "gtkrequest.h"
#include "gtkutils.h"
#include "pidgincore.h"

/******************************************************************************
 * Enums
 *****************************************************************************/

enum {
	AOP_ICON_COLUMN,
	AOP_NAME_COLUMN,
	AOP_DATA_COLUMN,
	AOP_COLUMN_COUNT
};

enum {
	COMPLETION_DISPLAYED_COLUMN,  /* displayed completion value */
	COMPLETION_BUDDY_COLUMN,      /* buddy name */
	COMPLETION_NORMALIZED_COLUMN, /* UTF-8 normalized & casefolded buddy name */
	COMPLETION_COMPARISON_COLUMN, /* UTF-8 normalized & casefolded value for comparison */
	COMPLETION_ACCOUNT_COLUMN,    /* account */
	COMPLETION_COLUMN_COUNT
};

/******************************************************************************
 * Structs
 *****************************************************************************/

typedef struct
{
	GtkWidget *entry;
	GtkWidget *accountopt;

	PidginFilterBuddyCompletionEntryFunc filter_func;
	gpointer filter_func_user_data;

	GtkListStore *store;
} PidginCompletionData;

/******************************************************************************
 * Code
 *****************************************************************************/

GtkWidget *
pidgin_make_frame(GtkWidget *parent, const char *title)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkLabel *label;
	char *labeltitle;

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_box_append(GTK_BOX(parent), vbox);

	label = GTK_LABEL(gtk_label_new(NULL));

	labeltitle = g_strdup_printf("<span weight=\"bold\">%s</span>", title);
	gtk_label_set_markup(label, labeltitle);
	g_free(labeltitle);

	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_label_set_yalign(GTK_LABEL(label), 0);
	gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(label));
	pidgin_set_accessible_label(vbox, label);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_box_append(GTK_BOX (vbox), hbox);

	label = GTK_LABEL(gtk_label_new("    "));
	gtk_box_append(GTK_BOX(hbox), GTK_WIDGET(label));

	vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_box_append(GTK_BOX(hbox), vbox2);

	g_object_set_data(G_OBJECT(vbox2), "main-vbox", vbox);

	return vbox2;
}

static void
aop_option_menu_select_by_data(GtkWidget *optmenu, gpointer data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gpointer iter_data;
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(optmenu));
	if (gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			gtk_tree_model_get(model, &iter, AOP_DATA_COLUMN, &iter_data, -1);
			if (iter_data == data) {
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(optmenu), &iter);
				return;
			}
		} while (gtk_tree_model_iter_next(model, &iter));
	}
}

static void
show_retrieveing_info(PurpleConnection *conn, const char *name)
{
	PurpleNotifyUserInfo *info = purple_notify_user_info_new();
	purple_notify_user_info_add_pair_plaintext(info, _("Information"), _("Retrieving..."));
	purple_notify_userinfo(conn, name, info, NULL, NULL);
	purple_notify_user_info_destroy(info);
}

void pidgin_retrieve_user_info(PurpleConnection *conn, const char *name)
{
	show_retrieveing_info(conn, name);
	purple_serv_get_info(conn, name);
}

void pidgin_retrieve_user_info_in_chat(PurpleConnection *conn, const char *name, int chat)
{
	char *who = NULL;
	PurpleProtocol *protocol = NULL;

	if (chat < 0) {
		pidgin_retrieve_user_info(conn, name);
		return;
	}

	protocol = purple_connection_get_protocol(conn);
	if (protocol != NULL)
		who = purple_protocol_chat_get_user_real_name(PURPLE_PROTOCOL_CHAT(protocol), conn, chat, name);

	pidgin_retrieve_user_info(conn, who ? who : name);
	g_free(who);
}

void
pidgin_set_accessible_label(GtkWidget *w, GtkLabel *l)
{
	pidgin_set_accessible_relations(w, l);
}

void
pidgin_set_accessible_relations (GtkWidget *w, GtkLabel *l)
{
	GtkAccessible *acc, *label;

	acc = GTK_ACCESSIBLE(w);
	label = GTK_ACCESSIBLE(l);

	/* Make sure mnemonics work */
	gtk_label_set_mnemonic_widget(l, w);

	/* Create the labeled-by relation */
	gtk_accessible_update_relation(acc, GTK_ACCESSIBLE_RELATION_LABELLED_BY,
	                               label, NULL, -1);
}

void pidgin_buddy_icon_get_scale_size(GdkPixbuf *buf, PurpleBuddyIconSpec *spec, PurpleBuddyIconScaleFlags rules, int *width, int *height)
{
	*width = gdk_pixbuf_get_width(buf);
	*height = gdk_pixbuf_get_height(buf);

	if ((spec == NULL) || !(spec->scale_rules & rules))
		return;

	purple_buddy_icon_spec_get_scaled_size(spec, width, height);

	/* and now for some arbitrary sanity checks */
	if(*width > 100)
		*width = 100;
	if(*height > 100)
		*height = 100;
}

static gboolean buddyname_completion_match_func(GtkEntryCompletion *completion,
		const gchar *key, GtkTreeIter *iter, gpointer user_data)
{
	GtkTreeModel *model;
	GValue val1;
	GValue val2;
	const char *tmp;

	model = gtk_entry_completion_get_model(completion);

	val1.g_type = 0;
	gtk_tree_model_get_value(model, iter, COMPLETION_NORMALIZED_COLUMN, &val1);
	tmp = g_value_get_string(&val1);
	if (tmp != NULL && g_str_has_prefix(tmp, key)) {
		g_value_unset(&val1);
		return TRUE;
	}
	g_value_unset(&val1);

	val2.g_type = 0;
	gtk_tree_model_get_value(model, iter, COMPLETION_COMPARISON_COLUMN, &val2);
	tmp = g_value_get_string(&val2);
	if (tmp != NULL && g_str_has_prefix(tmp, key)) {
		g_value_unset(&val2);
		return TRUE;
	}
	g_value_unset(&val2);

	return FALSE;
}

static gboolean buddyname_completion_match_selected_cb(GtkEntryCompletion *completion,
		GtkTreeModel *model, GtkTreeIter *iter, PidginCompletionData *data)
{
	GValue val;
	GtkWidget *optmenu = data->accountopt;
	PurpleAccount *account;

	val.g_type = 0;
	gtk_tree_model_get_value(model, iter, COMPLETION_BUDDY_COLUMN, &val);
	gtk_editable_set_text(GTK_EDITABLE(data->entry), g_value_get_string(&val));
	g_value_unset(&val);

	gtk_tree_model_get_value(model, iter, COMPLETION_ACCOUNT_COLUMN, &val);
	account = g_value_get_pointer(&val);
	g_value_unset(&val);

	if (account == NULL)
		return TRUE;

	if (optmenu != NULL)
		aop_option_menu_select_by_data(optmenu, account);

	return TRUE;
}

static void
add_buddyname_autocomplete_entry(GtkListStore *store, const char *buddy_alias, const char *contact_alias,
								  const PurpleAccount *account, const char *buddyname)
{
	GtkTreeIter iter;
	gboolean completion_added = FALSE;
	gchar *normalized_buddyname;
	gchar *tmp;

	tmp = g_utf8_normalize(buddyname, -1, G_NORMALIZE_DEFAULT);
	normalized_buddyname = g_utf8_casefold(tmp, -1);
	g_free(tmp);

	/* There's no sense listing things like: 'xxx "xxx"'
	   when the name and buddy alias match. */
	if (buddy_alias && !purple_strequal(buddy_alias, buddyname)) {
		char *completion_entry = g_strdup_printf("%s \"%s\"", buddyname, buddy_alias);
		char *tmp2 = g_utf8_normalize(buddy_alias, -1, G_NORMALIZE_DEFAULT);

		tmp = g_utf8_casefold(tmp2, -1);
		g_free(tmp2);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				COMPLETION_DISPLAYED_COLUMN, completion_entry,
				COMPLETION_BUDDY_COLUMN, buddyname,
				COMPLETION_NORMALIZED_COLUMN, normalized_buddyname,
				COMPLETION_COMPARISON_COLUMN, tmp,
				COMPLETION_ACCOUNT_COLUMN, account,
				-1);
		g_free(completion_entry);
		g_free(tmp);
		completion_added = TRUE;
	}

	/* There's no sense listing things like: 'xxx "xxx"'
	   when the name and contact alias match. */
	if (contact_alias && !purple_strequal(contact_alias, buddyname)) {
		/* We don't want duplicates when the contact and buddy alias match. */
		if (!purple_strequal(contact_alias, buddy_alias)) {
			char *completion_entry = g_strdup_printf("%s \"%s\"",
							buddyname, contact_alias);
			char *tmp2 = g_utf8_normalize(contact_alias, -1, G_NORMALIZE_DEFAULT);

			tmp = g_utf8_casefold(tmp2, -1);
			g_free(tmp2);

			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					COMPLETION_DISPLAYED_COLUMN, completion_entry,
					COMPLETION_BUDDY_COLUMN, buddyname,
					COMPLETION_NORMALIZED_COLUMN, normalized_buddyname,
					COMPLETION_COMPARISON_COLUMN, tmp,
					COMPLETION_ACCOUNT_COLUMN, account,
					-1);
			g_free(completion_entry);
			g_free(tmp);
			completion_added = TRUE;
		}
	}

	if (completion_added == FALSE) {
		/* Add the buddy's name. */
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				COMPLETION_DISPLAYED_COLUMN, buddyname,
				COMPLETION_BUDDY_COLUMN, buddyname,
				COMPLETION_NORMALIZED_COLUMN, normalized_buddyname,
				COMPLETION_COMPARISON_COLUMN, NULL,
				COMPLETION_ACCOUNT_COLUMN, account,
				-1);
	}

	g_free(normalized_buddyname);
}

static void
add_completion_list(PidginCompletionData *data)
{
	PurpleBlistNode *gnode, *cnode, *bnode;
	PidginFilterBuddyCompletionEntryFunc filter_func = data->filter_func;
	gpointer user_data = data->filter_func_user_data;
	gchar *alias;

	gtk_list_store_clear(data->store);

	for (gnode = purple_blist_get_default_root(); gnode != NULL;
	     gnode = gnode->next) {
		if (!PURPLE_IS_GROUP(gnode))
			continue;

		for (cnode = gnode->child; cnode != NULL; cnode = cnode->next)
		{
			if (!PURPLE_IS_META_CONTACT(cnode))
				continue;

			g_object_get(cnode, "alias", &alias, NULL);

			for (bnode = cnode->child; bnode != NULL; bnode = bnode->next)
			{
				PidginBuddyCompletionEntry entry;
				entry.is_buddy = TRUE;
				entry.buddy = (PurpleBuddy *) bnode;

				if (filter_func(&entry, user_data)) {
					add_buddyname_autocomplete_entry(data->store,
														alias,
														purple_buddy_get_contact_alias(entry.buddy),
														purple_buddy_get_account(entry.buddy),
														purple_buddy_get_name(entry.buddy)
													 );
				}
			}

			g_free(alias);
		}
	}
}

static void
buddyname_autocomplete_destroyed_cb(GtkWidget *widget, gpointer data)
{
	g_free(data);
	purple_signals_disconnect_by_handle(widget);
}

static void
repopulate_autocomplete(gpointer something, gpointer data)
{
	add_completion_list(data);
}

void
pidgin_setup_screenname_autocomplete(
        GtkWidget *entry, GtkWidget *chooser,
        PidginFilterBuddyCompletionEntryFunc filter_func, gpointer user_data)
{
	PidginCompletionData *data;

	/*
	 * Store the displayed completion value, the buddy name, the UTF-8
	 * normalized & casefolded buddy name, the UTF-8 normalized &
	 * casefolded value for comparison, and the account.
	 */
	GtkListStore *store;

	GtkEntryCompletion *completion;

	data = g_new0(PidginCompletionData, 1);
	store = gtk_list_store_new(COMPLETION_COLUMN_COUNT, G_TYPE_STRING,
	                           G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
	                           G_TYPE_POINTER);

	data->entry = entry;
	data->accountopt = chooser;
	if (filter_func == NULL) {
		data->filter_func = pidgin_screenname_autocomplete_default_filter;
		data->filter_func_user_data = NULL;
	} else {
		data->filter_func = filter_func;
		data->filter_func_user_data = user_data;
	}
	data->store = store;

	add_completion_list(data);

	/* Sort the completion list by buddy name */
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),
	                                     COMPLETION_BUDDY_COLUMN,
	                                     GTK_SORT_ASCENDING);

	completion = gtk_entry_completion_new();
	gtk_entry_completion_set_match_func(completion, buddyname_completion_match_func, NULL, NULL);

	g_signal_connect(G_OBJECT(completion), "match-selected",
		G_CALLBACK(buddyname_completion_match_selected_cb), data);

	gtk_entry_set_completion(GTK_ENTRY(entry), completion);
	g_object_unref(completion);

	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_entry_completion_set_text_column(completion, COMPLETION_DISPLAYED_COLUMN);

	purple_signal_connect(purple_connections_get_handle(), "signed-on", entry,
						G_CALLBACK(repopulate_autocomplete), data);
	purple_signal_connect(purple_connections_get_handle(), "signed-off", entry,
						G_CALLBACK(repopulate_autocomplete), data);

	purple_signal_connect(purple_accounts_get_handle(), "account-added", entry,
						G_CALLBACK(repopulate_autocomplete), data);
	purple_signal_connect(purple_accounts_get_handle(), "account-removed", entry,
						G_CALLBACK(repopulate_autocomplete), data);

	g_signal_connect(G_OBJECT(entry), "destroy", G_CALLBACK(buddyname_autocomplete_destroyed_cb), data);
}

gboolean
pidgin_screenname_autocomplete_default_filter(const PidginBuddyCompletionEntry *completion_entry, gpointer all_accounts) {
	gboolean all = GPOINTER_TO_INT(all_accounts);

	return all || purple_account_is_connected(purple_buddy_get_account(completion_entry->buddy));
}

/*
 * str_array_match:
 *
 * Returns: %TRUE if any string from array @a exists in array @b.
 */
static gboolean
str_array_match(char **a, char **b)
{
	int i, j;

	if (!a || !b)
		return FALSE;
	for (i = 0; a[i] != NULL; i++)
		for (j = 0; b[j] != NULL; j++)
			if (!g_ascii_strcasecmp(a[i], b[j]))
				return TRUE;
	return FALSE;
}

gpointer
pidgin_convert_buddy_icon(PurpleProtocol *protocol, const char *path, size_t *len)
{
	PurpleBuddyIconSpec *spec;
	int orig_width, orig_height, new_width, new_height;
	GdkPixbufFormat *format;
	char **pixbuf_formats;
	char **protocol_formats;
	GError *error = NULL;
	gchar *contents;
	gsize length;
	GdkPixbuf *pixbuf, *original;
	float scale_factor;
	int i;
	gchar *tmp;

	spec = purple_protocol_get_icon_spec(protocol);
	if(spec->format == NULL) {
		purple_buddy_icon_spec_free(spec);

		return NULL;
	}

	format = gdk_pixbuf_get_file_info(path, &orig_width, &orig_height);
	if (format == NULL) {
		purple_debug_warning("buddyicon", "Could not get file info of %s\n", path);

		purple_buddy_icon_spec_free(spec);

		return NULL;
	}

	pixbuf_formats = gdk_pixbuf_format_get_extensions(format);
	protocol_formats = g_strsplit(spec->format, ",", 0);

	if (str_array_match(pixbuf_formats, protocol_formats) && /* This is an acceptable format AND */
		 (!(spec->scale_rules & PURPLE_ICON_SCALE_SEND) || /* The protocol doesn't scale before it sends OR */
		  (spec->min_width <= orig_width && spec->max_width >= orig_width &&
		   spec->min_height <= orig_height && spec->max_height >= orig_height))) /* The icon is the correct size */
	{
		g_strfreev(pixbuf_formats);

		if (!g_file_get_contents(path, &contents, &length, &error)) {
			purple_debug_warning("buddyicon", "Could not get file contents "
					"of %s: %s\n", path, error->message);
			g_strfreev(protocol_formats);
			purple_buddy_icon_spec_free(spec);
			return NULL;
		}

		if (spec->max_filesize == 0 || length < spec->max_filesize) {
			/* The supplied image fits the file size, dimensions and type
			   constraints.  Great!  Return it without making any changes. */
			if (len)
				*len = length;
			g_strfreev(protocol_formats);
			purple_buddy_icon_spec_free(spec);
			return contents;
		}

		/* The image was too big.  Fall-through and try scaling it down. */
		g_free(contents);
	} else {
		g_strfreev(pixbuf_formats);
	}

	/* The original image wasn't compatible.  Scale it or convert file type. */
	pixbuf = gdk_pixbuf_new_from_file(path, &error);
	if (error) {
		purple_debug_warning("buddyicon", "Could not open icon '%s' for "
				"conversion: %s\n", path, error->message);
		g_error_free(error);
		g_strfreev(protocol_formats);
		purple_buddy_icon_spec_free(spec);
		return NULL;
	}
	original = g_object_ref(pixbuf);

	new_width = orig_width;
	new_height = orig_height;

	/* Make sure the image is the correct dimensions */
	if (spec->scale_rules & PURPLE_ICON_SCALE_SEND &&
		(orig_width < spec->min_width || orig_width > spec->max_width ||
		 orig_height < spec->min_height || orig_height > spec->max_height))
	{
		purple_buddy_icon_spec_get_scaled_size(spec, &new_width, &new_height);

		g_object_unref(G_OBJECT(pixbuf));
		pixbuf = gdk_pixbuf_scale_simple(original, new_width, new_height, GDK_INTERP_HYPER);
	}

	scale_factor = 1;
	do {
		for (i = 0; protocol_formats[i]; i++) {
			int quality = 100;
			do {
				const char *key = NULL;
				const char *value = NULL;
				gchar tmp_buf[4];

				purple_debug_info("buddyicon", "Converting buddy icon to %s\n", protocol_formats[i]);

				if (purple_strequal(protocol_formats[i], "png")) {
					key = "compression";
					value = "9";
				} else if (purple_strequal(protocol_formats[i], "jpeg")) {
					sprintf(tmp_buf, "%u", quality);
					key = "quality";
					value = tmp_buf;
				}

				if (!gdk_pixbuf_save_to_buffer(pixbuf, &contents, &length,
						protocol_formats[i], &error, key, value, NULL))
				{
					/* The NULL checking of error is necessary due to this bug:
					 * http://bugzilla.gnome.org/show_bug.cgi?id=405539 */
					purple_debug_warning("buddyicon",
							"Could not convert to %s: %s\n", protocol_formats[i],
							(error && error->message) ? error->message : "Unknown error");
					g_error_free(error);
					error = NULL;

					/* We couldn't convert to this image type.  Try the next
					   image type. */
					break;
				}

				if (spec->max_filesize == 0 || length <= spec->max_filesize) {
					/* We were able to save the image as this image type and
					   have it be within the size constraints.  Great!  Return
					   the image. */
					purple_debug_info("buddyicon", "Converted image from "
							"%dx%d to %dx%d, format=%s, quality=%u, "
							"filesize=%" G_GSIZE_FORMAT "\n",
							orig_width, orig_height, new_width, new_height,
							protocol_formats[i], quality, length);
					if (len)
						*len = length;
					g_strfreev(protocol_formats);
					g_object_unref(G_OBJECT(pixbuf));
					g_object_unref(G_OBJECT(original));
					purple_buddy_icon_spec_free(spec);
					return contents;
				}

				g_free(contents);

				if (!purple_strequal(protocol_formats[i], "jpeg")) {
					/* File size was too big and we can't lower the quality,
					   so skip to the next image type. */
					break;
				}

				/* File size was too big, but we're dealing with jpeg so try
				   lowering the quality. */
				quality -= 5;
			} while (quality >= 70);
		}

		/* We couldn't save the image in any format that was below the max
		   file size.  Maybe we can reduce the image dimensions? */
		scale_factor *= 0.8;
		new_width = orig_width * scale_factor;
		new_height = orig_height * scale_factor;
		g_object_unref(G_OBJECT(pixbuf));
		pixbuf = gdk_pixbuf_scale_simple(original, new_width, new_height, GDK_INTERP_HYPER);
	} while ((new_width > 10 || new_height > 10) && new_width > spec->min_width && new_height > spec->min_height);
	g_strfreev(protocol_formats);
	g_object_unref(G_OBJECT(pixbuf));
	g_object_unref(G_OBJECT(original));

	tmp = g_strdup_printf(_("The file '%s' is too large for %s.  Please try a smaller image.\n"),
			path, purple_protocol_get_name(protocol));
	purple_notify_error(NULL, _("Icon Error"), _("Could not set icon"), tmp, NULL);
	g_free(tmp);

	purple_buddy_icon_spec_free(spec);

	return NULL;
}

/*
 * "This is so dead sexy."
 * "Two thumbs up."
 * "Best movie of the year."
 *
 * This is the function that handles CTRL+F searching in the buddy list.
 * It finds the top-most buddy/group/chat/whatever containing the
 * entered string.
 *
 * It's somewhat ineffecient, because we strip all the HTML from the
 * "name" column of the buddy list (because the GtkTreeModel does not
 * contain the screen name in a non-markedup format).  But the alternative
 * is to add an extra column to the GtkTreeModel.  And this function is
 * used rarely, so it shouldn't matter TOO much.
 */
gboolean pidgin_tree_view_search_equal_func(GtkTreeModel *model, gint column,
			const gchar *key, GtkTreeIter *iter, gpointer data)
{
	gchar *enteredstring;
	gchar *tmp;
	gchar *withmarkup;
	gchar *nomarkup;
	gchar *normalized;
	gboolean result;
	size_t i;
	size_t len;
	PangoLogAttr *log_attrs;
	gchar *word;

	if (g_ascii_strcasecmp(key, "Global Thermonuclear War") == 0)
	{
		purple_notify_info(NULL, "WOPR", "Wouldn't you prefer a nice "
			"game of chess?", NULL, NULL);
		return FALSE;
	}

	gtk_tree_model_get(model, iter, column, &withmarkup, -1);
	if (withmarkup == NULL)   /* This is probably a separator */
		return TRUE;

	tmp = g_utf8_normalize(key, -1, G_NORMALIZE_DEFAULT);
	enteredstring = g_utf8_casefold(tmp, -1);
	g_free(tmp);

	nomarkup = purple_markup_strip_html(withmarkup);
	tmp = g_utf8_normalize(nomarkup, -1, G_NORMALIZE_DEFAULT);
	g_free(nomarkup);
	normalized = g_utf8_casefold(tmp, -1);
	g_free(tmp);

	if (g_str_has_prefix(normalized, enteredstring)) {
		g_free(withmarkup);
		g_free(enteredstring);
		g_free(normalized);
		return FALSE;
	}

	/* Use Pango to separate by words. */
	len = g_utf8_strlen(normalized, -1);
	log_attrs = g_new(PangoLogAttr, len + 1);

	pango_get_log_attrs(normalized, strlen(normalized), -1, NULL, log_attrs, len + 1);

	word = normalized;
	result = TRUE;
	for (i = 0; i < (len - 1) ; i++)
	{
		if (log_attrs[i].is_word_start &&
		    g_str_has_prefix(word, enteredstring)) {
			result = FALSE;
			break;
		}
		word = g_utf8_next_char(word);
	}
	g_free(log_attrs);

/* The non-Pango version. */
#if 0
	word = normalized;
	result = TRUE;
	while (word[0] != '\0')
	{
		gunichar c = g_utf8_get_char(word);
		if (!g_unichar_isalnum(c))
		{
			word = g_utf8_find_next_char(word, NULL);
			if (g_str_has_prefix(word, enteredstring))
			{
				result = FALSE;
				break;
			}
		}
		else
			word = g_utf8_find_next_char(word, NULL);
	}
#endif

	g_free(withmarkup);
	g_free(enteredstring);
	g_free(normalized);

	return result;
}

GtkWidget *
pidgin_add_widget_to_vbox(GtkBox *vbox, const char *widget_label, GtkSizeGroup *sg, GtkWidget *widget, gboolean expand, GtkWidget **p_label)
{
	GtkWidget *hbox;
	GtkWidget *label = NULL;

	if (widget_label) {
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
		gtk_box_append(vbox, hbox);

		label = gtk_label_new_with_mnemonic(widget_label);
		if (sg) {
			gtk_label_set_xalign(GTK_LABEL(label), 0);
			gtk_size_group_add_widget(sg, label);
		}
		gtk_box_append(GTK_BOX(hbox), label);

		gtk_widget_set_hexpand(widget, expand);
		gtk_box_append(GTK_BOX(hbox), widget);
	} else {
		gtk_widget_set_vexpand(widget, expand);
		gtk_box_append(vbox, widget);

		hbox = GTK_WIDGET(vbox);
	}

	if (label) {
		gtk_label_set_mnemonic_widget(GTK_LABEL(label), widget);
		pidgin_set_accessible_label(widget, GTK_LABEL(label));
	}

	if (p_label)
		(*p_label) = label;
	return hbox;
}

gboolean pidgin_auto_parent_window(GtkWidget *widget)
{
	/* This finds the currently active window and makes that the parent window. */
	GList *windows = NULL;
	GtkWindow *parent = NULL;
	gpointer parent_from;
	PurpleNotifyType notify_type;

	parent_from = g_object_get_data(G_OBJECT(widget), "pidgin-parent-from");
	if (purple_request_is_valid_ui_handle(parent_from, NULL)) {
		
		gtk_window_set_transient_for(GTK_WINDOW(widget),
			gtk_window_get_transient_for(
				pidgin_request_get_dialog_window(parent_from)));
		return TRUE;
	}
	if (purple_notify_is_valid_ui_handle(parent_from, &notify_type) &&
		notify_type == PURPLE_NOTIFY_MESSAGE)
	{
		gtk_window_set_transient_for(GTK_WINDOW(widget),
			gtk_window_get_transient_for(GTK_WINDOW(parent_from)));
		return TRUE;
	}

	windows = gtk_window_list_toplevels();
	while (windows) {
		GtkWindow *window = GTK_WINDOW(windows->data);
		windows = g_list_delete_link(windows, windows);

		if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(window),
			"pidgin-window-is-closing")))
		{
			parent = gtk_window_get_transient_for(window);
			break;
		}

		if (GTK_WIDGET(window) == widget ||
				!gtk_widget_get_visible(GTK_WIDGET(window))) {
			continue;
		}

		if (gtk_window_is_active(window)) {
			parent = window;
			break;
		}
	}
	if (windows)
		g_list_free(windows);
	if (parent) {
		gtk_window_set_transient_for(GTK_WINDOW(widget), parent);
		return TRUE;
	}
	return FALSE;
}
