/**
 * @file gg.c Gadu-Gadu protocol plugin
 *
 * gaim
 *
 * Copyright (C) 2005  Bartosz Oler <bartosz@bzimage.us>
 *
 * Some parts of the code are adapted or taken from the previous implementation
 * of this plugin written by Arkadiusz Miskiewicz <misiek@pld.org.pl>
 *
 * Thanks to Google's Summer of Code Program.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "internal.h"

#include "plugin.h"
#include "version.h"
#include "notify.h"
#include "status.h"
#include "blist.h"
#include "accountopt.h"
#include "debug.h"
#include "util.h"
#include "request.h"

#include "lib/libgadu.h"

#include "gg.h"
#include "confer.h"
#include "search.h"
#include "buddylist.h"
#include "utils.h"

static GaimPlugin *my_protocol = NULL;

/* ---------------------------------------------------------------------- */
/* ----- EXTERNAL CALLBACKS --------------------------------------------- */
/* ---------------------------------------------------------------------- */


/* ----- HELPERS -------------------------------------------------------- */

/*
 */
/* static void ggp_async_token_handler(gpointer _gc, gint fd, GaimInputCondition cond) {{{ */
static void ggp_async_token_handler(gpointer _gc, gint fd, GaimInputCondition cond)
{
	GaimConnection *gc = _gc;
	GGPInfo *info = gc->proto_data;
	GGPToken *token = info->token;
	GGPTokenCallback cb;

	struct gg_token *t = NULL;

	gaim_debug_info("gg", "token_handler: token->req->fd = %d\n", token->req->fd);
	gaim_debug_info("gg", "token_handler: token->req: check = %d; state = %d;\n",
			token->req->check, token->req->state);

	if (gg_token_watch_fd(token->req) == -1 || token->req->state == GG_STATE_ERROR) {
		gaim_debug_error("gg", "token error (1): %d\n", token->req->error);
		gaim_input_remove(token->inpa);
		gg_token_free(token->req);
		token->req = NULL;

		gaim_notify_error(gaim_connection_get_account(gc),
				  _("Token Error"),
				  _("Unable to fetch the token.\n"), NULL);
		return;
	}

	if (token->req->state != GG_STATE_DONE) {
		gaim_input_remove(token->inpa);
		token->inpa = gaim_input_add(token->req->fd,
						   (token->req->check == 1)
						   	? GAIM_INPUT_WRITE
							: GAIM_INPUT_READ,
						   ggp_async_token_handler, gc);
		return;
	}

	if (!(t = token->req->data) || !token->req->body) {
		gaim_debug_error("gg", "token error (2): %d\n", token->req->error);
		gaim_input_remove(token->inpa);
		gg_token_free(token->req);
		token->req = NULL;

		gaim_notify_error(gaim_connection_get_account(gc),
				  _("Token Error"),
				  _("Unable to fetch the token.\n"), NULL);
		return;
	}

	gaim_input_remove(token->inpa);

	token->id = g_strdup(t->tokenid);
	token->size = token->req->body_size;
	token->data = g_new0(char, token->size);
	memcpy(token->data, token->req->body, token->size);

	gaim_debug_info("gg", "TOKEN! tokenid = %s; size = %d\n",
			token->id, token->size);

	gg_token_free(token->req);
	token->req = NULL;
	token->inpa = 0;

	cb = token->cb;
	token->cb = NULL;
	cb(gc);
}
/* }}} */

/*
 */
/* static void ggp_token_request(GaimConnection *gc, GGPTokenCallback cb) {{{ */
static void ggp_token_request(GaimConnection *gc, GGPTokenCallback cb)
{
	GGPInfo *info = gc->proto_data;
	struct gg_http *req;

	if ((req = gg_token(1)) == NULL) {
		gaim_notify_error(gaim_connection_get_account(gc),
				  _("Token Error"),
				  _("Unable to fetch the token.\n"), NULL);
		return;
	}

	info->token = g_new(GGPToken, 1);
	info->token->cb = cb;

	info->token->req = req;
	info->token->inpa = gaim_input_add(req->fd, GAIM_INPUT_READ,
					   ggp_async_token_handler, gc);
}
/* }}} */

/* ---------------------------------------------------------------------- */

/**
 * Request buddylist from the server.
 * Buddylist is received in the ggp_callback_recv().
 *
 * @param Current action handler.
 */
/* static void ggp_action_buddylist_get(GaimPluginAction *action) {{{ */
static void ggp_action_buddylist_get(GaimPluginAction *action)
{
	GaimConnection *gc = (GaimConnection *)action->context;
	GGPInfo *info = gc->proto_data;

	gaim_debug_info("gg", "Downloading...\n");

	gg_userlist_request(info->session, GG_USERLIST_GET, NULL);
}
/* }}} */

/**
 * Upload the buddylist to the server.
 *
 * @param action Current action handler.
 */
/* static void ggp_action_buddylist_put(GaimPluginAction *action) {{{ */
static void ggp_action_buddylist_put(GaimPluginAction *action)
{
	GaimConnection *gc = (GaimConnection *)action->context;
	GGPInfo *info = gc->proto_data;

	char *buddylist = ggp_buddylist_dump(gaim_connection_get_account(gc));

	gaim_debug_info("gg", "Uploading...\n");
	
	if (buddylist == NULL)
		return;

	gg_userlist_request(info->session, GG_USERLIST_PUT, buddylist);
	g_free(buddylist);
}
/* }}} */

/**
 * Delete buddylist from the server.
 *
 * @param action Current action handler.
 */
/* static void ggp_action_buddylist_delete(GaimPluginAction *action) {{{ */
static void ggp_action_buddylist_delete(GaimPluginAction *action)
{
	GaimConnection *gc = (GaimConnection *)action->context;
	GGPInfo *info = gc->proto_data;

	gaim_debug_info("gg", "Deleting...\n");

	gg_userlist_request(info->session, GG_USERLIST_PUT, NULL);
}
/* }}} */

/*
 */
/* static void ggp_callback_buddylist_save_ok(GaimConnection *gc, gchar *file) {{{ */
static void ggp_callback_buddylist_save_ok(GaimConnection *gc, gchar *file)
{
	GaimAccount *account = gaim_connection_get_account(gc);

	FILE *fh;
	char *buddylist = ggp_buddylist_dump(account);
	gchar *msg;

	gaim_debug_info("gg", "Saving...\n");
	gaim_debug_info("gg", "file = %s\n", file);

	if (buddylist == NULL) {
		gaim_notify_info(account, _("Save Buddylist..."),
			 _("Your buddylist is empty, nothing was written to the file."),
			 NULL);
		return;
	}

	if ((fh = g_fopen(file, "wb")) == NULL) {
		msg = g_strconcat(_("Couldn't open file"), ": ", file, "\n", NULL);
		gaim_debug_error("gg", "Could not open file: %s\n", file);
		gaim_notify_error(account, _("Couldn't open file"), msg, NULL);
		g_free(msg);
		g_free(file);
		return;
	}

	fwrite(buddylist, sizeof(char), g_utf8_strlen(buddylist, -1), fh);
	fclose(fh);
	g_free(buddylist);

	gaim_notify_info(account, _("Save Buddylist..."),
			 _("Buddylist saved successfully!"), NULL);
}
/* }}} */

/*
 */
/* static void ggp_callback_buddylist_load_ok(GaimConnection *gc, gchar *file) {{{ */
static void ggp_callback_buddylist_load_ok(GaimConnection *gc, gchar *file)
{
	GaimAccount *account = gaim_connection_get_account(gc);
	GError *error = NULL;
	char *buddylist = NULL;
	gsize length;

	gaim_debug_info("gg", "file_name = %s\n", file);

	if (!g_file_get_contents(file, &buddylist, &length, &error)) {
		gaim_notify_error(account,
				_("Couldn't load buddylist"),
				_("Couldn't load buddylist"),
				error->message);

		gaim_debug_error("gg",
			"Couldn't load buddylist. file = %s; error = %s\n",
			file, error->message);

		g_error_free(error);

		return;
	}

	ggp_buddylist_load(gc, buddylist);
	g_free(buddylist);

	gaim_notify_info(account,
			 _("Load Buddylist..."),
			 _("Buddylist loaded successfully!"), NULL);
}
/* }}} */

/*
 */
/* static void ggp_action_buddylist_save(GaimPluginAction *action) {{{ */
static void ggp_action_buddylist_save(GaimPluginAction *action)
{
	GaimConnection *gc = (GaimConnection *)action->context;

	gaim_request_file(action, _("Save buddylist..."), NULL, TRUE,
			G_CALLBACK(ggp_callback_buddylist_save_ok), NULL, gc);
}
/* }}} */

/*
 */
/* static void ggp_action_buddylist_load(GaimPluginAction *action) {{{ */
static void ggp_action_buddylist_load(GaimPluginAction *action)
{
	GaimConnection *gc = (GaimConnection *)action->context;

	gaim_request_file(action, "Load buddylist from file...", NULL, FALSE,
			G_CALLBACK(ggp_callback_buddylist_load_ok), NULL, gc);
}
/* }}} */

/*
 */
/* static void ggp_callback_register_account_ok(GaimConnection *gc, GaimRequestFields *fields) {{{ */
static void ggp_callback_register_account_ok(GaimConnection *gc,
					     GaimRequestFields *fields)
{
	GaimAccount *account;
	GGPInfo *info = gc->proto_data;
	struct gg_http *h = NULL;
	struct gg_pubdir *s;
	uin_t uin;
	gchar *email, *p1, *p2, *t;
	GGPToken *token = info->token;

	email = charset_convert(gaim_request_fields_get_string(fields, "email"),
			     "UTF-8", "CP1250");
	p1  = charset_convert(gaim_request_fields_get_string(fields, "password1"),
			     "UTF-8", "CP1250");
	p2  = charset_convert(gaim_request_fields_get_string(fields, "password2"),
			     "UTF-8", "CP1250");
	t   = charset_convert(gaim_request_fields_get_string(fields, "token"),
			     "UTF-8", "CP1250");

	account = gaim_connection_get_account(gc);

	if (email == NULL || p1 == NULL || p2 == NULL || t == NULL ||
	    *email == '\0' || *p1 == '\0' || *p2 == '\0' || *t == '\0') {
		gaim_connection_error(gc, _("Fill in the registration fields."));
		goto exit_err;
	}

	if (g_utf8_collate(p1, p2) != 0) {
		gaim_connection_error(gc, _("Passwords do not match."));
		goto exit_err;
	}

	gaim_debug_info("gg", "register_account_ok: token_id = %d; t = %s\n",
			token->id, t);
	h = gg_register3(email, p1, token->id, t, 0);
	if (h == NULL || !(s = h->data) || !s->success) {
		gaim_connection_error(gc,
			_("Unable to register new account. Error occurred.\n"));
		goto exit_err;
	}

	uin = s->uin;
	gaim_debug_info("gg", "registered uin: %d\n", uin);

	g_free(t);
	t = g_strdup_printf("%u", uin);
	gaim_account_set_username(account, t);
	/* Save the password if remembering passwords for the account */
	gaim_account_set_password(account, p1);

	gaim_notify_info(NULL, _("New Gadu-Gadu Account Registered"),
			 _("Registration completed successfully!"), NULL);

	/* TODO: the currently open Accounts Window will not be updated withthe
	 * new username and etc, we need to somehow have it refresh at this
	 * point
	 */

	/* Need to disconnect or actually log in. For now, we disconnect. */
	gaim_connection_destroy(gc);

exit_err:
	gg_register_free(h);
	g_free(email);
	g_free(p1);
	g_free(p2);
	g_free(t);
	g_free(token->id);
	g_free(token);
}
/* }}} */

/*
 */
/* static void ggp_callback_register_account_cancel(GaimConnection *gc, GaimRequestFields *fields) {{{ */
static void ggp_callback_register_account_cancel(GaimConnection *gc,
						 GaimRequestFields *fields)
{
	GGPInfo *info = gc->proto_data;
	GGPToken *token = info->token;

	gaim_connection_destroy(gc);

	g_free(token->id);
	g_free(token->data);
	g_free(token);

}
/* }}} */

/*
 */
/* static void ggp_register_user_dialog(GaimConnection *gc) {{{ */
static void ggp_register_user_dialog(GaimConnection *gc)
{
	GaimAccount *account;
	GaimRequestFields *fields;
	GaimRequestFieldGroup *group;
	GaimRequestField *field;

	GGPInfo *info = gc->proto_data;
	GGPToken *token = info->token;


	account = gaim_connection_get_account(gc);

	fields = gaim_request_fields_new();
	group = gaim_request_field_group_new(NULL);
	gaim_request_fields_add_group(fields, group);

	field = gaim_request_field_string_new("email",
			_("e-Mail"), "", FALSE);
	gaim_request_field_string_set_masked(field, FALSE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("password1",
			_("Password"), "", FALSE);
	gaim_request_field_string_set_masked(field, TRUE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("password2",
			_("Password (retype)"), "", FALSE);
	gaim_request_field_string_set_masked(field, TRUE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("token",
			_("Enter current token"), "", FALSE);
	gaim_request_field_string_set_masked(field, FALSE);
	gaim_request_field_group_add_field(group, field);

	/* original size: 60x24 */
	field = gaim_request_field_image_new("token_img",
			_("Current token"), token->data, token->size);
	gaim_request_field_group_add_field(group, field);

	gaim_request_fields(account,
		_("Register New Gadu-Gadu Account"),
		_("Register New Gadu-Gadu Account"),
		_("Please, fill in the following fields"),
		fields,
		_("OK"), G_CALLBACK(ggp_callback_register_account_ok),
		_("Cancel"), G_CALLBACK(ggp_callback_register_account_cancel),
		gc);
}
/* }}} */

/* ----- PUBLIC DIRECTORY SEARCH ---------------------------------------- */

/*
 */
/* static void ggp_callback_show_next(GaimConnection *gc, GList *row) {{{ */
static void ggp_callback_show_next(GaimConnection *gc, GList *row)
{
	GGPInfo *info = gc->proto_data;

	g_free(info->search_form->offset);
	info->search_form->offset = g_strdup(info->search_form->last_uin);
	ggp_search_start(gc, info->search_form);
}
/* }}} */

/*
 */
/* static void ggp_callback_add_buddy(GaimConnection *gc, GList *row) {{{ */
static void ggp_callback_add_buddy(GaimConnection *gc, GList *row)
{
	gaim_blist_request_add_buddy(gaim_connection_get_account(gc),
				     g_list_nth_data(row, 0), NULL, NULL);
}
/* }}} */

/*
 */
/* static void ggp_callback_find_buddies(GaimConnection *gc, GaimRequestFields *fields) {{{ */
static void ggp_callback_find_buddies(GaimConnection *gc, GaimRequestFields *fields)
{
	GGPInfo *info = gc->proto_data;
	GGPSearchForm *form;

	form = ggp_search_form_new();
	info->search_form = form;

	form->lastname  = charset_convert(
				gaim_request_fields_get_string(fields, "lastname"),
				"UTF-8", "CP1250");
	form->firstname = charset_convert(
				gaim_request_fields_get_string(fields, "firstname"),
				"UTF-8", "CP1250");
	form->nickname  = charset_convert(
				gaim_request_fields_get_string(fields, "nickname"),
				"UTF-8", "CP1250");
	form->city      = charset_convert(
				gaim_request_fields_get_string(fields, "city"),
				"UTF-8", "CP1250");
	form->birthyear = charset_convert(
				gaim_request_fields_get_string(fields, "year"),
				"UTF-8", "CP1250");

	switch (gaim_request_fields_get_choice(fields, "gender")) {
		case 1:
			form->gender = g_strdup(GG_PUBDIR50_GENDER_MALE);
			break;
		case 2:
			form->gender = g_strdup(GG_PUBDIR50_GENDER_FEMALE);
			break;
		default:
			form->gender = NULL;
			break;
	}

	form->active = gaim_request_fields_get_bool(fields, "active")
				   ? g_strdup(GG_PUBDIR50_ACTIVE_TRUE) : NULL;

	form->offset = g_strdup("0");

	ggp_search_start(gc, form);
}
/* }}} */

/*
 */
/* static void ggp_find_buddies(GaimPluginAction *action) {{{ */
static void ggp_find_buddies(GaimPluginAction *action)
{
	GaimConnection *gc = (GaimConnection *)action->context;
	GGPInfo *info = gc->proto_data;

	GaimRequestFields *fields;
	GaimRequestFieldGroup *group;
	GaimRequestField *field;

	if (info->search_form != NULL) {
		gaim_notify_error(gc, NULL,
			_("Unable to initiate a new search"),
			_("You have a pending search. Please wait for it to finish."));
		return;
	}

	fields = gaim_request_fields_new();
	group = gaim_request_field_group_new(NULL);
	gaim_request_fields_add_group(fields, group);

	field = gaim_request_field_string_new("lastname",
			_("Last name"), NULL, FALSE);
	gaim_request_field_string_set_masked(field, FALSE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("firstname",
			_("First name"), NULL, FALSE);
	gaim_request_field_string_set_masked(field, FALSE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("nickname",
			_("Nickname"), NULL, FALSE);
	gaim_request_field_string_set_masked(field, FALSE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("city",
			_("City"), NULL, FALSE);
	gaim_request_field_string_set_masked(field, FALSE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("year",
			_("Year of birth"), NULL, FALSE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_choice_new("gender", _("Gender"), 0);
	gaim_request_field_choice_add(field, _("Male or female"));
	gaim_request_field_choice_add(field, _("Male"));
	gaim_request_field_choice_add(field, _("Female"));
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_bool_new("active",
			_("Only online"), FALSE);
	gaim_request_field_group_add_field(group, field);

	gaim_request_fields(gc,
		_("Find buddies"),
		_("Find buddies"),
		_("Please, enter your search criteria below"),
		fields,
		_("OK"), G_CALLBACK(ggp_callback_find_buddies),
		_("Cancel"), NULL,
		gc);
}
/* }}} */

/* ----- CHANGE PASSWORD ------------------------------------------------ */

/*
 */
/* static void ggp_callback_change_passwd_ok(GaimConnection *gc, GaimRequestFields *fields) {{{ */
static void ggp_callback_change_passwd_ok(GaimConnection *gc, GaimRequestFields *fields)
{
	GaimAccount *account;
	GGPInfo *info = gc->proto_data;
	struct gg_http *h;
	gchar *cur, *p1, *p2, *t;

	cur = charset_convert(
			gaim_request_fields_get_string(fields, "password_cur"),
			"UTF-8", "CP1250");
	p1  = charset_convert(
			gaim_request_fields_get_string(fields, "password1"),
			"UTF-8", "CP1250");
	p2  = charset_convert(
			gaim_request_fields_get_string(fields, "password2"),
			"UTF-8", "CP1250");
	t   = charset_convert(
			gaim_request_fields_get_string(fields, "token"),
			"UTF-8", "CP1250");

	account = gaim_connection_get_account(gc);

	if (cur == NULL || p1 == NULL || p2 == NULL || t == NULL ||
	    *cur == '\0' || *p1 == '\0' || *p2 == '\0' || *t == '\0') {
		gaim_notify_error(account, NULL, _("Fill in the fields."), NULL);
		goto exit_err;
	}

	if (g_utf8_collate(p1, p2) != 0) {
		gaim_notify_error(account, NULL,
				  _("New passwords do not match."), NULL);
		goto exit_err;
	}

	if (g_utf8_collate(cur, gaim_account_get_password(account)) != 0) {
		gaim_notify_error(account, NULL,
			_("Your current password is different from the one that you specified."),
			NULL);
		goto exit_err;
	}

	gaim_debug_info("gg", "Changing password\n");

	/* XXX: this e-mail should be a pref... */
	h = gg_change_passwd4(ggp_get_uin(account),
			      "user@example.net", gaim_account_get_password(account),
			      p1, info->token->id, t, 0);

	if (h == NULL) {
		gaim_notify_error(account, NULL,
			_("Unable to change password. Error occured.\n"),
			NULL);
		goto exit_err;
	}

	gaim_account_set_password(account, p1);

	gg_change_passwd_free(h);

	gaim_notify_info(account, _("Change password for the Gadu-Gadu account"),
			 _("Password was changed successfully!"), NULL);

exit_err:
	g_free(cur);
	g_free(p1);
	g_free(p2);
	g_free(t);
	g_free(info->token->id);
	g_free(info->token->data);
	g_free(info->token);
}
/* }}} */

/*
 */
/* static void ggp_change_passwd_dialog(GaimConnection *gc) {{{ */
static void ggp_change_passwd_dialog(GaimConnection *gc)
{
	GaimRequestFields *fields;
	GaimRequestFieldGroup *group;
	GaimRequestField *field;

	GGPInfo *info = gc->proto_data;
	GGPToken *token = info->token;

	char *msg;


	fields = gaim_request_fields_new();
	group = gaim_request_field_group_new(NULL);
	gaim_request_fields_add_group(fields, group);

	field = gaim_request_field_string_new("password_cur",
			_("Current password"), "", FALSE);
	gaim_request_field_string_set_masked(field, TRUE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("password1",
			_("Password"), "", FALSE);
	gaim_request_field_string_set_masked(field, TRUE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("password2",
			_("Password (retype)"), "", FALSE);
	gaim_request_field_string_set_masked(field, TRUE);
	gaim_request_field_group_add_field(group, field);

	field = gaim_request_field_string_new("token",
			_("Enter current token"), "", FALSE);
	gaim_request_field_string_set_masked(field, FALSE);
	gaim_request_field_group_add_field(group, field);

	/* original size: 60x24 */
	field = gaim_request_field_image_new("token_img",
			_("Current token"), token->data, token->size);
	gaim_request_field_group_add_field(group, field);

	msg = g_strdup_printf("%s %d",
		_("Please, enter your current password and your new password for UIN: "),
		ggp_get_uin(gaim_connection_get_account(gc)));

	gaim_request_fields(gc,
		_("Change Gadu-Gadu Password"),
		_("Change Gadu-Gadu Password"),
		msg,
		fields, _("OK"), G_CALLBACK(ggp_callback_change_passwd_ok),
		_("Cancel"), NULL, gc);

	g_free(msg);
}
/* }}} */

/*
 */
/* static void ggp_change_passwd(GaimPluginAction *action) {{{ */
static void ggp_change_passwd(GaimPluginAction *action)
{
	GaimConnection *gc = (GaimConnection *)action->context;

	ggp_token_request(gc, ggp_change_passwd_dialog);
}
/* }}} */

/* ----- CONFERENCES ---------------------------------------------------- */

/*
 */
/* static void ggp_callback_add_to_chat_ok(GaimConnection *gc, GaimRequestFields *fields) {{{ */
static void ggp_callback_add_to_chat_ok(GaimConnection *gc, GaimRequestFields *fields)
{
	GGPInfo *info = gc->proto_data;
	GaimRequestField *field;
	const GList *sel;

	field = gaim_request_fields_get_field(fields, "name");
	sel = gaim_request_field_list_get_selected(field);

	ggp_confer_participants_add_uin(gc, sel->data, info->tmp_buddy);
	info->tmp_buddy = 0;
}
/* }}} */

/*
 */
/* static void ggp_bmenu_add_to_chat(GaimBlistNode *node, gpointer ignored) {{{ */
static void ggp_bmenu_add_to_chat(GaimBlistNode *node, gpointer ignored)
{
	GaimBuddy *buddy;
	GaimConnection *gc;
	GGPInfo *info;

	GaimRequestFields *fields;
	GaimRequestFieldGroup *group;
	GaimRequestField *field;

	GList *l;
	gchar *msg;

	buddy = (GaimBuddy *)node;
	gc = gaim_account_get_connection(gaim_buddy_get_account(buddy));
	info = gc->proto_data;

	/* TODO: It tmp_buddy != 0 then stop! */
	info->tmp_buddy = ggp_str_to_uin(gaim_buddy_get_name(buddy));

	fields = gaim_request_fields_new();
	group = gaim_request_field_group_new(NULL);
	gaim_request_fields_add_group(fields, group);

	field = gaim_request_field_list_new("name", "Chat name");
	for (l = info->chats; l != NULL; l = l->next) {
		GGPChat *chat = l->data;
		gaim_request_field_list_add(field, g_strdup(chat->name),
					    g_strdup(chat->name));
	}
	gaim_request_field_group_add_field(group, field);

	/* TODO: s/screenname/alias/ */
	msg = g_strdup_printf(_("Select a chat for buddy: %s"),
			      gaim_buddy_get_name(buddy));
	gaim_request_fields(gc,
			_("Add to chat..."),
			_("Add to chat..."),
			msg,
			fields,
			_("Add"), G_CALLBACK(ggp_callback_add_to_chat_ok),
			_("Cancel"), NULL, gc);
	g_free(msg);
}
/* }}} */

/* ----- BLOCK BUDDIES -------------------------------------------------- */

/*
 */
/* static void ggp_bmenu_block(GaimBlistNode *node, gpointer ignored) {{{ */
static void ggp_bmenu_block(GaimBlistNode *node, gpointer ignored)
{
	GaimConnection *gc;
	GaimBuddy *buddy;
	GGPInfo *info;
	uin_t uin;

	buddy = (GaimBuddy *)node;
	gc = gaim_account_get_connection(gaim_buddy_get_account(buddy));
	info = gc->proto_data;

	uin = ggp_str_to_uin(gaim_buddy_get_name(buddy));

	if (gaim_blist_node_get_bool(node, "blocked")) {
		gaim_blist_node_set_bool(node, "blocked", FALSE);
		gg_remove_notify_ex(info->session, uin, GG_USER_BLOCKED);
		gg_add_notify_ex(info->session, uin, GG_USER_NORMAL);
		gaim_debug_info("gg", "send: uin=%d; mode=NORMAL\n", uin);
	} else {
		gaim_blist_node_set_bool(node, "blocked", TRUE);
		gg_remove_notify_ex(info->session, uin, GG_USER_NORMAL);
		gg_add_notify_ex(info->session, uin, GG_USER_BLOCKED);
		gaim_debug_info("gg", "send: uin=%d; mode=BLOCKED\n", uin);
	}
}
/* }}} */

/* ---------------------------------------------------------------------- */
/* ----- INTERNAL CALLBACKS --------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* just a prototype */
static void ggp_set_status(GaimAccount *account, GaimStatus *status);

/**
 * Handle change of the status of the buddy.
 *
 * @param gc     GaimConnection
 * @param uin    UIN of the buddy.
 * @param status ID of the status.
 * @param descr  Description.
 */
/* static void ggp_generic_status_handler(GaimConnection *gc, uin_t uin, int status, const char *descr) {{{ */
static void ggp_generic_status_handler(GaimConnection *gc, uin_t uin,
				       int status, const char *descr)
{
	gchar *from;
	const char *st;
	gchar *msg;

	from = g_strdup_printf("%ld", (unsigned long int)uin);
	switch (status) {
		case GG_STATUS_NOT_AVAIL:
		case GG_STATUS_NOT_AVAIL_DESCR:
			st = "offline";
			break;
		case GG_STATUS_AVAIL:
		case GG_STATUS_AVAIL_DESCR:
			st = "available";
			break;
		case GG_STATUS_BUSY:
		case GG_STATUS_BUSY_DESCR:
			st = "away";
			break;
		case GG_STATUS_BLOCKED:
			/* user is blocking us.... */
			st = "blocked";
			break;
		default:
			st = "available";
			gaim_debug_info("gg",
				"GG_EVENT_NOTIFY: Unknown status: %d\n", status);
			break;
	}

	gaim_debug_info("gg", "st = %s\n", st);
	msg = charset_convert(descr, "CP1250", "UTF-8");
	gaim_prpl_got_user_status(gaim_connection_get_account(gc),
				  from, st, "message", msg, NULL);
	g_free(from);
	g_free(msg);
}
/* }}} */

/*
 */
/* static void ggp_sr_close_cb(GaimAccount *account) {{{ */
static void ggp_sr_close_cb(GaimAccount *account)
{
	GaimConnection *gc = gaim_account_get_connection(account);
	GGPInfo *info = gc->proto_data;
	GGPSearchForm *f;

	info->searchresults_window = NULL;

	f = info->search_form;
	g_free(f->uin);
	g_free(f->lastname);
	g_free(f->firstname);
	g_free(f->nickname);
	g_free(f->city);
	g_free(f->birthyear);
	g_free(f->gender);
	g_free(f->active);
	g_free(f->offset);
	g_free(f->last_uin);
	g_free(f);

	info->search_form = NULL;
}
/* }}} */

/*
 */
/* static void ggp_pubdir_reply_handler(GaimConnection *gc, gg_pubdir50_t req) {{{ */
static void ggp_pubdir_reply_handler(GaimConnection *gc, gg_pubdir50_t req)
{
	GGPInfo *info = gc->proto_data;
	GaimNotifySearchResults *results;
	GaimNotifySearchColumn *column;
	int res_count = 0;
	int start;
	int i;

	res_count = gg_pubdir50_count(req);
	if (res_count < 1) {
		gaim_debug_info("gg", "GG_EVENT_PUBDIR50_SEARCH_REPLY: Nothing found\n");
		gaim_notify_error(gc, NULL,
			_("No matching users found"),
			_("There are no users matching your search criteria."));
		return;
	}
	res_count = (res_count > 20) ? 20 : res_count;

	results = gaim_notify_searchresults_new();

	if (results == NULL) {
		gaim_debug_error("gg", "ggp_pubdir_reply_handler: "
				 "Unable to display the search results.\n");
		gaim_notify_error(gc, NULL,
				  _("Unable to display the search results."),
				  NULL);
		ggp_sr_close_cb(gaim_connection_get_account(gc));
		return;
	}

	column = gaim_notify_searchresults_column_new(_("UIN"));
	gaim_notify_searchresults_column_add(results, column);

	column = gaim_notify_searchresults_column_new(_("First name"));
	gaim_notify_searchresults_column_add(results, column);

	column = gaim_notify_searchresults_column_new(_("Nickname"));
	gaim_notify_searchresults_column_add(results, column);

	column = gaim_notify_searchresults_column_new(_("City"));
	gaim_notify_searchresults_column_add(results, column);

	column = gaim_notify_searchresults_column_new(_("Birth year"));
	gaim_notify_searchresults_column_add(results, column);

	gaim_debug_info("gg", "Going with %d entries\n", res_count);

	start = (int)ggp_str_to_uin(gg_pubdir50_get(req, 0, GG_PUBDIR50_START));
	gaim_debug_info("gg", "start = %d\n", start);

	for (i = 0; i < res_count; i++) {
		GList *row = NULL;
		char *birth = ggp_search_get_result(req, i, GG_PUBDIR50_BIRTHYEAR);

		/* TODO: Status will be displayed as an icon. */
		/* row = g_list_append(row, ggp_search_get_result(req, i, GG_PUBDIR50_STATUS)); */
		row = g_list_append(row, ggp_search_get_result(req, i,
							GG_PUBDIR50_UIN));
		row = g_list_append(row, ggp_search_get_result(req, i,
							GG_PUBDIR50_FIRSTNAME));
		row = g_list_append(row, ggp_search_get_result(req, i,
							GG_PUBDIR50_NICKNAME));
		row = g_list_append(row, ggp_search_get_result(req, i,
							GG_PUBDIR50_CITY));
		row = g_list_append(row,
			(birth && strncmp(birth, "0", 1)) ? birth : g_strdup("-"));

		gaim_notify_searchresults_row_add(results, row);

		if (i == res_count - 1) {
			g_free(info->search_form->last_uin);
			info->search_form->last_uin = ggp_search_get_result(req, i,
								GG_PUBDIR50_UIN);
		}
	}

	gaim_notify_searchresults_button_add(results, GAIM_NOTIFY_BUTTON_CONTINUE,
					     ggp_callback_show_next);
	gaim_notify_searchresults_button_add(results, GAIM_NOTIFY_BUTTON_ADD,
					     ggp_callback_add_buddy);
	if (info->searchresults_window == NULL) {
		void *h = gaim_notify_searchresults(gc,
				_("Gadu-Gadu Public Directory"),
				_("Search results"), NULL, results,
				(GaimNotifyCloseCallback)ggp_sr_close_cb,
				gaim_connection_get_account(gc));

		if (h == NULL) {
			gaim_debug_error("gg", "ggp_pubdir_reply_handler: "
					 "Unable to display the search results.\n");
			gaim_notify_error(gc, NULL,
					  _("Unable to display the search results."),
					  NULL);
			ggp_sr_close_cb(gaim_connection_get_account(gc));
			return;
		}

		info->searchresults_window = h;
	} else {
		gaim_notify_searchresults_new_rows(gc, results,
				info->searchresults_window, NULL);
	}
}
/* }}} */

/**
 * Dispatch a message received from a buddy.
 *
 * @param gc GaimConnection.
 * @param ev Gadu-Gadu event structure.
 */
/* static void ggp_recv_message_handler(GaimConnection *gc, const struct gg_event *ev) {{{ */
static void ggp_recv_message_handler(GaimConnection *gc, const struct gg_event *ev)
{
	GGPInfo *info = gc->proto_data;
	GaimConversation *conv;
	gchar *from;
	gchar *msg;
	gchar *tmp;

	from = g_strdup_printf("%lu", (unsigned long int)ev->event.msg.sender);

	tmp = charset_convert((const char *)ev->event.msg.message,
			      "CP1250", "UTF-8");
	gaim_str_strip_char(tmp, '\r');
	msg = g_markup_escape_text(tmp, -1);
	g_free(tmp);

	gaim_debug_info("gg", "msg form (%s): %s (class = %d; rcpt_count = %d)\n",
			from, msg, ev->event.msg.msgclass,
			ev->event.msg.recipients_count);

	if (ev->event.msg.recipients_count == 0) {
		serv_got_im(gc, from, msg, 0, ev->event.msg.time);
	} else {
		const char *chat_name;
		int chat_id;
		char *buddy_name;

		chat_name = ggp_confer_find_by_participants(gc,
				ev->event.msg.recipients,
				ev->event.msg.recipients_count);

		if (chat_name == NULL) {
			chat_name = ggp_confer_add_new(gc, NULL);
			serv_got_joined_chat(gc, info->chats_count, chat_name);

			ggp_confer_participants_add_uin(gc, chat_name,
							ev->event.msg.sender);

			ggp_confer_participants_add(gc, chat_name,
						    ev->event.msg.recipients,
						    ev->event.msg.recipients_count);
		}
		conv = ggp_confer_find_by_name(gc, chat_name);
		chat_id = gaim_conv_chat_get_id(GAIM_CONV_CHAT(conv));

		buddy_name = ggp_buddy_get_name(gc, ev->event.msg.sender);
		serv_got_chat_in(gc, chat_id, buddy_name,
				 GAIM_MESSAGE_RECV, msg, ev->event.msg.time);
		g_free(buddy_name);
	}
	g_free(msg);
	g_free(from);
}
/* }}} */

/*
 */
/* static void ggp_callback_recv(gpointer _gc, gint fd, GaimInputCondition cond) {{{ */
static void ggp_callback_recv(gpointer _gc, gint fd, GaimInputCondition cond)
{
	GaimConnection *gc = _gc;
	GGPInfo *info = gc->proto_data;
	struct gg_event *ev;
	int i;

	if (!(ev = gg_watch_fd(info->session))) {
		gaim_debug_error("gg",
			"ggp_callback_recv: gg_watch_fd failed -- CRITICAL!\n");
		gaim_connection_error(gc, _("Unable to read socket"));
		return;
	}

	switch (ev->type) {
		case GG_EVENT_NONE:
			/* Nothing happened. */
			break;
		case GG_EVENT_MSG:
			ggp_recv_message_handler(gc, ev);
			break;
		case GG_EVENT_ACK:
			gaim_debug_info("gg",
				"message sent to: %ld, delivery status=%d, seq=%d\n",
				ev->event.ack.recipient, ev->event.ack.status,
				ev->event.ack.seq);
			break;
		case GG_EVENT_NOTIFY:
		case GG_EVENT_NOTIFY_DESCR:
			{
				struct gg_notify_reply *n;
				char *descr;

				gaim_debug_info("gg", "notify_pre: (%d) status: %d\n",
						ev->event.notify->uin,
						ev->event.notify->status);

				n = (ev->type == GG_EVENT_NOTIFY) ? ev->event.notify
								  : ev->event.notify_descr.notify;

				for (; n->uin; n++) {
					descr = (ev->type == GG_EVENT_NOTIFY) ? NULL
							: ev->event.notify_descr.descr;

					gaim_debug_info("gg",
						"notify: (%d) status: %d; descr: %s\n",
						n->uin, n->status, descr);

					ggp_generic_status_handler(gc,
						n->uin, n->status, descr);
				}
			}
			break;
		case GG_EVENT_NOTIFY60:
			gaim_debug_info("gg",
				"notify60_pre: (%d) status=%d; version=%d; descr=%s\n",
				ev->event.notify60->uin, ev->event.notify60->status,
				ev->event.notify60->version, ev->event.notify60->descr);

			for (i = 0; ev->event.notify60[i].uin; i++) {
				gaim_debug_info("gg",
					"notify60: (%d) status=%d; version=%d; descr=%s\n",
					ev->event.notify60[i].uin,
					ev->event.notify60[i].status,
					ev->event.notify60[i].version,
					ev->event.notify60[i].descr);

				ggp_generic_status_handler(gc, ev->event.notify60[i].uin,
					ev->event.notify60[i].status,
					ev->event.notify60[i].descr);
			}
			break;
		case GG_EVENT_STATUS:
			gaim_debug_info("gg", "status: (%d) status=%d; descr=%s\n",
					ev->event.status.uin, ev->event.status.status,
					ev->event.status.descr);

			ggp_generic_status_handler(gc, ev->event.status.uin,
				ev->event.status.status, ev->event.status.descr);
			break;
		case GG_EVENT_STATUS60:
			gaim_debug_info("gg",
				"status60: (%d) status=%d; version=%d; descr=%s\n",
				ev->event.status60.uin, ev->event.status60.status,
				ev->event.status60.version, ev->event.status60.descr);

			ggp_generic_status_handler(gc, ev->event.status60.uin,
				ev->event.status60.status, ev->event.status60.descr);
			break;
		case GG_EVENT_USERLIST:
	    		if (ev->event.userlist.type == GG_USERLIST_GET_REPLY) {
				gaim_debug_info("gg", "GG_USERLIST_GET_REPLY\n");
				gaim_notify_info(gc, NULL,
					_("Buddy list downloaded"),
					_("Your buddy list was downloaded from the server."));
				if (ev->event.userlist.reply != NULL) {
					ggp_buddylist_load(gc, ev->event.userlist.reply);
				}
			} else {
				gaim_debug_info("gg", "GG_USERLIST_PUT_REPLY\n");
				gaim_notify_info(gc, NULL,
					_("Buddy list uploaded"),
					_("Your buddy list was stored on the server."));
			}
			break;
		case GG_EVENT_PUBDIR50_SEARCH_REPLY:
			ggp_pubdir_reply_handler(gc, ev->event.pubdir50);
			break;
		default:
			gaim_debug_error("gg",
				"unsupported event type=%d\n", ev->type);
			break;
	}

	gg_free_event(ev);
}
/* }}} */

/*
 */
/* static void ggp_async_login_handler(gpointer _gc, gint fd, GaimInputCondition cond) {{{ */
static void ggp_async_login_handler(gpointer _gc, gint fd, GaimInputCondition cond)
{
	GaimConnection *gc = _gc;
	GGPInfo *info = gc->proto_data;
	struct gg_event *ev;

	gaim_debug_info("gg", "login_handler: session: check = %d; state = %d;\n",
			info->session->check, info->session->state);

	switch (info->session->state) {
		case GG_STATE_RESOLVING:
			gaim_debug_info("gg", "GG_STATE_RESOLVING\n");
			break;
		case GG_STATE_CONNECTING_HUB:
			gaim_debug_info("gg", "GG_STATE_CONNECTING_HUB\n");
			break;
		case GG_STATE_READING_DATA:
			gaim_debug_info("gg", "GG_STATE_READING_DATA\n");
			break;
		case GG_STATE_CONNECTING_GG:
			gaim_debug_info("gg", "GG_STATE_CONNECTING_GG\n");
			break;
		case GG_STATE_READING_KEY:
			gaim_debug_info("gg", "GG_STATE_READING_KEY\n");
			break;
		case GG_STATE_READING_REPLY:
			gaim_debug_info("gg", "GG_STATE_READING_REPLY\n");
			break;
		default:
			gaim_debug_error("gg", "unknown state = %d\n",
					 info->session->state);
		break;
	}

	if (!(ev = gg_watch_fd(info->session))) {
		gaim_debug_error("gg", "login_handler: gg_watch_fd failed!\n");
		gaim_connection_error(gc, _("Unable to read socket"));
		return;
	}
	gaim_debug_info("gg", "login_handler: session->fd = %d\n", info->session->fd);
	gaim_debug_info("gg", "login_handler: session: check = %d; state = %d;\n",
			info->session->check, info->session->state);

	gaim_input_remove(gc->inpa);

	/** XXX I think that this shouldn't be done if ev->type is GG_EVENT_CONN_FAILED or GG_EVENT_CONN_SUCCESS -datallah */
	gc->inpa = gaim_input_add(info->session->fd,
				  (info->session->check == 1) ? GAIM_INPUT_WRITE
							      : GAIM_INPUT_READ,
				  ggp_async_login_handler, gc);

	switch (ev->type) {
		case GG_EVENT_NONE:
			/* Nothing happened. */
			gaim_debug_info("gg", "GG_EVENT_NONE\n");
			break;
		case GG_EVENT_CONN_SUCCESS:
			{
				GaimAccount *account;
				GaimPresence *presence;
				GaimStatus *status;

				gaim_debug_info("gg", "GG_EVENT_CONN_SUCCESS\n");
				gaim_input_remove(gc->inpa);
				gc->inpa = gaim_input_add(info->session->fd,
							  GAIM_INPUT_READ,
							  ggp_callback_recv, gc);

				/* gg_change_status(info->session, GG_STATUS_AVAIL); */

				account = gaim_connection_get_account(gc);
				presence = gaim_account_get_presence(account);
				status = gaim_presence_get_active_status(presence);

				ggp_set_status(account, status);
				gaim_connection_set_state(gc, GAIM_CONNECTED);
				ggp_buddylist_send(gc);
			}
			break;
		case GG_EVENT_CONN_FAILED:
			gaim_input_remove(gc->inpa);
			gc->inpa = 0;
			gaim_connection_error(gc, _("Connection failed."));
			break;
		default:
			gaim_debug_error("gg", "strange event: %d\n", ev->type);
			break;
	}

	gg_free_event(ev);
}
/* }}} */

/* ---------------------------------------------------------------------- */
/* ----- GaimPluginProtocolInfo ----------------------------------------- */
/* ---------------------------------------------------------------------- */

/* static const char *ggp_list_icon(GaimAccount *account, GaimBuddy *buddy) {{{ */
static const char *ggp_list_icon(GaimAccount *account, GaimBuddy *buddy)
{
	return "gadu-gadu";
}
/* }}} */

/* static void ggp_list_emblems(GaimBuddy *b, const char **se, const char **sw, const char **nw, const char **ne) {{{ */
static void ggp_list_emblems(GaimBuddy *b, const char **se, const char **sw,
					   const char **nw, const char **ne)
{
	GaimPresence *presence = gaim_buddy_get_presence(b);

	/* 
	 * Note to myself:
	 * 	The only valid status types are those defined
	 * 	in prpl_info->status_types.
	 *
	 * Usable icons: away, blocked, dnd, extended_away,
	 * freeforchat, ignored, invisible, na, offline.
	 */

	if (!GAIM_BUDDY_IS_ONLINE(b)) {
		*se = "offline";
	} else if (gaim_presence_is_status_primitive_active(presence, GAIM_STATUS_AWAY)) {
		*se = "away";
	} else if (gaim_presence_is_status_active(presence, "blocked")) {
		*se = "blocked";
	}
}
/* }}} */

/* static char *ggp_status_text(GaimBuddy *b) {{{ */
static char *ggp_status_text(GaimBuddy *b)
{
	GaimStatus *status;
	const char *msg;
	char *text;
	char *tmp;

	status = gaim_presence_get_active_status(gaim_buddy_get_presence(b));

	msg = gaim_status_get_attr_string(status, "message");

	if (msg != NULL) {
		tmp = gaim_markup_strip_html(msg);
		text = g_markup_escape_text(tmp, -1);
		g_free(tmp);

		return text;
	} else {
		tmp = g_strdup(gaim_status_get_name(status));
		text = g_markup_escape_text(tmp, -1);
		g_free(tmp);

		return text;
	}
}
/* }}} */

/* static char *ggp_tooltip_text(GaimBuddy *b, gboolean full) {{{ */
static char *ggp_tooltip_text(GaimBuddy *b, gboolean full)
{
	GaimStatus *status;
	char *text;
	gchar *ret;
	const char *msg, *name;

	status = gaim_presence_get_active_status(gaim_buddy_get_presence(b));
	msg = gaim_status_get_attr_string(status, "message");
	name = gaim_status_get_name(status);

	if (msg != NULL) {
		char *tmp = gaim_markup_strip_html(msg);
		text = g_markup_escape_text(tmp, -1);
		g_free(tmp);

		ret = g_strdup_printf("\n<b>%s:</b> %s: %s",
				      _("Status"), name, text);

		g_free(text);
	} else {
		ret = g_strdup_printf("\n<b>%s:</b> %s",
				      _("Status"), name);
	}

	return ret;
}
/* }}} */

/* static GList *ggp_status_types(GaimAccount *account) {{{ */
static GList *ggp_status_types(GaimAccount *account)
{
	GaimStatusType *type;
	GList *types = NULL;

	type = gaim_status_type_new_with_attrs(
			GAIM_STATUS_AVAILABLE, NULL, NULL, TRUE, TRUE, FALSE,
			"message", _("Message"), gaim_value_new(GAIM_TYPE_STRING),
			NULL);
	types = g_list_append(types, type);

	/*
	 * Without this selecting Invisible as own status doesn't
	 * work. It's not used and not needed to show status of buddies.
	 */
	type = gaim_status_type_new_with_attrs(
			GAIM_STATUS_INVISIBLE, NULL, NULL, TRUE, TRUE, FALSE,
			"message", _("Message"), gaim_value_new(GAIM_TYPE_STRING),
			NULL);
	types = g_list_append(types, type);

	type = gaim_status_type_new_with_attrs(
			GAIM_STATUS_AWAY, NULL, NULL, TRUE, TRUE, FALSE,
			"message", _("Message"), gaim_value_new(GAIM_TYPE_STRING),
			NULL);
	types = g_list_append(types, type);

	/*
	 * This status is necessary to display guys who are blocking *us*.
	 */
	type = gaim_status_type_new_with_attrs(
			GAIM_STATUS_INVISIBLE, "blocked", _("Blocked"), TRUE, FALSE, FALSE,
			"message", _("Message"), gaim_value_new(GAIM_TYPE_STRING), NULL);
	types = g_list_append(types, type);

	type = gaim_status_type_new_with_attrs(
			GAIM_STATUS_OFFLINE, NULL, NULL, TRUE, TRUE, FALSE,
			"message", _("Message"), gaim_value_new(GAIM_TYPE_STRING),
			NULL);
	types = g_list_append(types, type);

	return types;
}
/* }}} */

/* static GList *ggp_blist_node_menu(GaimBlistNode *node) {{{ */
static GList *ggp_blist_node_menu(GaimBlistNode *node)
{
	GaimMenuAction *act;
	GList *m = NULL;

	if (!GAIM_BLIST_NODE_IS_BUDDY(node))
		return NULL;

	act = gaim_menu_action_new(_("Add to chat"),
	                           GAIM_CALLBACK(ggp_bmenu_add_to_chat),
	                           NULL, NULL);
	m = g_list_append(m, act);

	/* Using a blist node boolean here is also wrong.
	 * Once the Block and Unblock actions are added to the core,
	 * this will have to go. -- rlaager */
	if (gaim_blist_node_get_bool(node, "blocked")) {
		act = gaim_menu_action_new(_("Unblock"),
		                           GAIM_CALLBACK(ggp_bmenu_block),
		                           NULL, NULL);
	} else {
		act = gaim_menu_action_new(_("Block"),
		                           GAIM_CALLBACK(ggp_bmenu_block),
		                           NULL, NULL);
	}
	m = g_list_append(m, act);

	return m;
}
/* }}} */

/* static GList *ggp_chat_info(GaimConnection *gc) {{{ */
static GList *ggp_chat_info(GaimConnection *gc)
{
	GList *m = NULL;
	struct proto_chat_entry *pce;

	pce = g_new0(struct proto_chat_entry, 1);
	pce->label = _("Chat _name:");
	pce->identifier = "name";
	pce->required = TRUE;
	m = g_list_append(m, pce);

	return m;
}
/* }}} */

/* static void ggp_login(GaimAccount *account) {{{ */
static void ggp_login(GaimAccount *account)
{
	GaimConnection *gc = gaim_account_get_connection(account);
	struct gg_login_params *glp = g_new0(struct gg_login_params, 1);
	GGPInfo *info = g_new0(GGPInfo, 1);

	/* Probably this should be moved to *_new() function. */
	info->session = NULL;
	info->searchresults_window = NULL;
	info->chats = NULL;
	info->chats_count = 0;
	info->token = NULL;

	gc->proto_data = info;

	glp->uin = ggp_get_uin(account);
	glp->password = (char *)gaim_account_get_password(account);

	glp->async = 1;
	glp->status = GG_STATUS_AVAIL;
	glp->tls = 0;

	info->session = gg_login(glp);
	if (info->session == NULL) {
		gaim_connection_error(gc, _("Connection failed."));
		g_free(glp);
		return;
	}
	gc->inpa = gaim_input_add(info->session->fd, GAIM_INPUT_READ,
				  ggp_async_login_handler, gc);
}
/* }}} */

/* static void ggp_close(GaimConnection *gc) {{{ */
static void ggp_close(GaimConnection *gc)
{

	if (gc == NULL) {
		gaim_debug_info("gg", "gc == NULL\n");
		return;
	}

	if (gc->proto_data) {
		GaimAccount *account = gaim_connection_get_account(gc);
		GaimStatus *status;
		GGPInfo *info = gc->proto_data;

		status = gaim_account_get_active_status(account);

		if (info->session != NULL) {
			ggp_set_status(account, status);
			gg_logoff(info->session);
			gg_free_session(info->session);
		}
		g_free(info);
		gc->proto_data = NULL;
	}

	if (gc->inpa > 0)
		gaim_input_remove(gc->inpa);

	ggp_buddylist_offline(gc);

	gaim_debug_info("gg", "Connection closed.\n");
}
/* }}} */

/* static int ggp_send_im(GaimConnection *gc, const char *who, const char *msg, GaimMessageFlags flags) {{{ */
static int ggp_send_im(GaimConnection *gc, const char *who, const char *msg,
		       GaimMessageFlags flags)
{
	GGPInfo *info = gc->proto_data;
	char *tmp, *plain;

	if (strlen(msg) == 0)
		return 1;

	plain = gaim_unescape_html(msg);
	gaim_debug_info("gg", "ggp_send_im: msg = %s\n", msg);
	tmp = charset_convert(plain, "UTF-8", "CP1250");
	g_free(plain);

	if (tmp != NULL && strlen(tmp) > 0) {
		if (gg_send_message(info->session, GG_CLASS_CHAT,
				ggp_str_to_uin(who), (unsigned char *)tmp) < 0) {
			return -1;
		}
	}
	g_free(tmp);

	return 1;
}
/* }}} */

/* static void ggp_get_info(GaimConnection *gc, const char *name) { {{{ */
static void ggp_get_info(GaimConnection *gc, const char *name)
{
	GGPInfo *info = gc->proto_data;
	GGPSearchForm *form;

	if (info->search_form != NULL) {
		gaim_notify_error(gc, NULL,
			_("Unable to initiate a new search"),
			_("You have a pending search. Please wait for it to finish."));
		return;
	}

	form = ggp_search_form_new();
	info->search_form = form;

	form->uin = g_strdup(name);
	form->offset = g_strdup("0");
	form->last_uin = g_strdup("0");

	ggp_search_start(gc, form);
}
/* }}} */

/* static void ggp_set_status(GaimAccount *account, GaimStatus *status) {{{ */
static void ggp_set_status(GaimAccount *account, GaimStatus *status)
{
	GaimConnection *gc;
	GGPInfo *info;
	const char *status_id, *msg;
	int new_status, new_status_descr;

	if (!gaim_status_is_active(status))
		return;

	gc = gaim_account_get_connection(account);
	info = gc->proto_data;

	status_id = gaim_status_get_id(status);

	gaim_debug_info("gg", "ggp_set_status: Requested status = %s\n",
			status_id);

	if (strcmp(status_id, "available") == 0) {
		new_status = GG_STATUS_AVAIL;
		new_status_descr = GG_STATUS_AVAIL_DESCR;
	} else if (strcmp(status_id, "away") == 0) {
		new_status = GG_STATUS_BUSY;
		new_status_descr = GG_STATUS_BUSY_DESCR;
	} else if (strcmp(status_id, "invisible") == 0) {
		new_status = GG_STATUS_INVISIBLE;
		new_status_descr = GG_STATUS_INVISIBLE_DESCR;
	} else if (strcmp(status_id, "offline") == 0) {
		new_status = GG_STATUS_NOT_AVAIL;
		new_status_descr = GG_STATUS_NOT_AVAIL_DESCR;
	} else {
		new_status = GG_STATUS_AVAIL;
		new_status_descr = GG_STATUS_AVAIL_DESCR;
		gaim_debug_info("gg",
			"ggp_set_status: uknown status requested (status_id=%s)\n",
			status_id);
	}

	msg = gaim_status_get_attr_string(status, "message");

	if (msg == NULL) {
		gg_change_status(info->session, new_status);
	} else {
		gchar *tmp, *new_msg;

		tmp = gaim_markup_strip_html(msg);
		new_msg = g_markup_escape_text(tmp, -1);
		g_free(tmp);

		tmp = charset_convert(new_msg, "UTF-8", "CP1250");
		gg_change_status_descr(info->session, new_status_descr, tmp);
		g_free(new_msg);
		g_free(tmp);
	}
}
/* }}} */

/* static void ggp_add_buddy(GaimConnection *gc, GaimBuddy *buddy, GaimGroup *group) {{{ */
static void ggp_add_buddy(GaimConnection *gc, GaimBuddy *buddy, GaimGroup *group)
{
	GGPInfo *info = gc->proto_data;

	gg_add_notify(info->session, ggp_str_to_uin(buddy->name));
}
/* }}} */

/* static void ggp_remove_buddy(GaimConnection *gc, GaimBuddy *buddy, GaimGroup *group) {{{ */
static void ggp_remove_buddy(GaimConnection *gc, GaimBuddy *buddy,
						 GaimGroup *group)
{
	GGPInfo *info = gc->proto_data;

	gg_remove_notify(info->session, ggp_str_to_uin(buddy->name));
}
/* }}} */

/* static void ggp_join_chat(GaimConnection *gc, GHashTable *data) {{{ */
static void ggp_join_chat(GaimConnection *gc, GHashTable *data)
{
	GGPInfo *info = gc->proto_data;
	GGPChat *chat;
	char *chat_name;
	GList *l;
	GaimConversation *conv;
	GaimAccount *account = gaim_connection_get_account(gc);

	chat_name = g_hash_table_lookup(data, "name");

	if (chat_name == NULL)
		return;

	gaim_debug_info("gg", "joined %s chat\n", chat_name);

	for (l = info->chats; l != NULL; l = l->next) {
		 chat = l->data;

		 if (chat != NULL && g_utf8_collate(chat->name, chat_name) == 0) {
			 gaim_notify_error(gc, _("Chat error"),
				 _("This chat name is already in use"), NULL);
			 return;
		 }
	}

	ggp_confer_add_new(gc, chat_name);
	conv = serv_got_joined_chat(gc, info->chats_count, chat_name);
	gaim_conv_chat_add_user(GAIM_CONV_CHAT(conv),
				gaim_account_get_username(account), NULL,
				GAIM_CBFLAGS_NONE, TRUE);
}
/* }}} */

/* static char *ggp_get_chat_name(GHashTable *data) { {{{ */
static char *ggp_get_chat_name(GHashTable *data) {
	return g_strdup(g_hash_table_lookup(data, "name"));
}
/* }}} */

/* static int ggp_chat_send(GaimConnection *gc, int id, const char *message, GaimMessageFlags flags) {{{ */
static int ggp_chat_send(GaimConnection *gc, int id, const char *message, GaimMessageFlags flags)
{
	GaimConversation *conv;
	GGPInfo *info = gc->proto_data;
	GGPChat *chat = NULL;
	GList *l;
	char *msg, *plain;
	uin_t *uins;
	int count = 0;

	if ((conv = gaim_find_chat(gc, id)) == NULL)
		return -EINVAL;

	for (l = info->chats; l != NULL; l = l->next) {
		chat = l->data;

		if (g_utf8_collate(chat->name, conv->name) == 0) {
			break;
		}

		chat = NULL;
	}

	if (chat == NULL) {
		gaim_debug_error("gg",
			"ggp_chat_send: Hm... that's strange. No such chat?\n");
		return -EINVAL;
	}

	uins = g_new0(uin_t, g_list_length(chat->participants));

	for (l = chat->participants; l != NULL; l = l->next) {
		uin_t uin = GPOINTER_TO_INT(l->data);

		uins[count++] = uin;
	}

	plain = gaim_unescape_html(message);
	msg = charset_convert(plain, "UTF-8", "CP1250");
	g_free(plain);
	gg_send_message_confer(info->session, GG_CLASS_CHAT, count, uins,
				(unsigned char *)msg);
	g_free(msg);
	g_free(uins);

	serv_got_chat_in(gc, id,
			 gaim_account_get_username(gaim_connection_get_account(gc)),
			 0, message, time(NULL));

	return 0;
}
/* }}} */

/* static void ggp_keepalive(GaimConnection *gc) {{{ */
static void ggp_keepalive(GaimConnection *gc)
{
	GGPInfo *info = gc->proto_data;

	/* gaim_debug_info("gg", "Keeping connection alive....\n"); */

	if (gg_ping(info->session) < 0) {
		gaim_debug_info("gg", "Not connected to the server "
				"or gg_session is not correct\n");
		gaim_connection_error(gc, _("Not connected to the server."));
	}
}
/* }}} */

/* static void ggp_register_user(GaimAccount *account) {{{ */
static void ggp_register_user(GaimAccount *account)
{
	GaimConnection *gc = gaim_account_get_connection(account);
	GGPInfo *info;

	info = gc->proto_data = g_new0(GGPInfo, 1);

	ggp_token_request(gc, ggp_register_user_dialog);
}
/* }}} */

/* static GList *ggp_actions(GaimPlugin *plugin, gpointer context) {{{ */
static GList *ggp_actions(GaimPlugin *plugin, gpointer context)
{
	GList *m = NULL;
	GaimPluginAction *act;

	act = gaim_plugin_action_new(_("Find buddies..."),
				     ggp_find_buddies);
	m = g_list_append(m, act);

	m = g_list_append(m, NULL);

	act = gaim_plugin_action_new(_("Change password..."),
				     ggp_change_passwd);
	m = g_list_append(m, act);

	m = g_list_append(m, NULL);

	act = gaim_plugin_action_new(_("Upload buddylist to Server"),
				     ggp_action_buddylist_put);
	m = g_list_append(m, act);

	act = gaim_plugin_action_new(_("Download buddylist from Server"),
				     ggp_action_buddylist_get);
	m = g_list_append(m, act);

	act = gaim_plugin_action_new(_("Delete buddylist from Server"),
				     ggp_action_buddylist_delete);
	m = g_list_append(m, act);

	act = gaim_plugin_action_new(_("Save buddylist to file..."),
				     ggp_action_buddylist_save);
	m = g_list_append(m, act);

	act = gaim_plugin_action_new(_("Load buddylist from file..."),
				     ggp_action_buddylist_load);
	m = g_list_append(m, act);

	return m;
}
/* }}} */

/* prpl_info setup {{{ */
static GaimPluginProtocolInfo prpl_info =
{
	OPT_PROTO_REGISTER_NOSCREENNAME,
	NULL,				/* user_splits */
	NULL,				/* protocol_options */
	NO_BUDDY_ICONS,			/* icon_spec */
	ggp_list_icon,			/* list_icon */
	ggp_list_emblems,		/* list_emblems */
	ggp_status_text,		/* status_text */
	ggp_tooltip_text,		/* tooltip_text */
	ggp_status_types,		/* status_types */
	ggp_blist_node_menu,		/* blist_node_menu */
	ggp_chat_info,			/* chat_info */
	NULL,				/* chat_info_defaults */
	ggp_login,			/* login */
	ggp_close,			/* close */
	ggp_send_im,			/* send_im */
	NULL,				/* set_info */
	NULL,				/* send_typing */
	ggp_get_info,			/* get_info */
	ggp_set_status,			/* set_away */
	NULL,				/* set_idle */
	NULL,				/* change_passwd */
	ggp_add_buddy,			/* add_buddy */
	NULL,				/* add_buddies */
	ggp_remove_buddy,		/* remove_buddy */
	NULL,				/* remove_buddies */
	NULL,				/* add_permit */
	NULL,				/* add_deny */
	NULL,				/* rem_permit */
	NULL,				/* rem_deny */
	NULL,				/* set_permit_deny */
	ggp_join_chat,			/* join_chat */
	NULL,				/* reject_chat */
	ggp_get_chat_name,		/* get_chat_name */
	NULL,				/* chat_invite */
	NULL,				/* chat_leave */
	NULL,				/* chat_whisper */
	ggp_chat_send,			/* chat_send */
	ggp_keepalive,			/* keepalive */
	ggp_register_user,		/* register_user */
	NULL,				/* get_cb_info */
	NULL,				/* get_cb_away */
	NULL,				/* alias_buddy */
	NULL,				/* group_buddy */
	NULL,				/* rename_group */
	NULL,				/* buddy_free */
	NULL,				/* convo_closed */
	NULL,				/* normalize */
	NULL,				/* set_buddy_icon */
	NULL,				/* remove_group */
	NULL,				/* get_cb_real_name */
	NULL,				/* set_chat_topic */
	NULL,				/* find_blist_chat */
	NULL,				/* roomlist_get_list */
	NULL,				/* roomlist_cancel */
	NULL,				/* roomlist_expand_category */
	NULL,				/* can_receive_file */
	NULL,				/* send_file */
	NULL,				/* new_xfer */
	NULL,				/* offline_message */
	NULL,				/* whiteboard_prpl_ops */
	NULL,				/* media_prpl_ops */
};
/* }}} */

/* GaimPluginInfo setup {{{ */
static GaimPluginInfo info = {
	GAIM_PLUGIN_MAGIC,		/* magic */
	GAIM_MAJOR_VERSION,		/* major_version */
	GAIM_MINOR_VERSION,		/* minor_version */
	GAIM_PLUGIN_PROTOCOL,		/* plugin type */
	NULL,				/* ui_requirement */
	0,				/* flags */
	NULL,				/* dependencies */
	GAIM_PRIORITY_DEFAULT,		/* priority */

	"prpl-gg",			/* id */
	"Gadu-Gadu",			/* name */
	VERSION,			/* version */

	N_("Gadu-Gadu Protocol Plugin"),	/* summary */
	N_("Polish popular IM"),		/* description */
	"boler@sourceforge.net",	/* author */
	GAIM_WEBSITE,			/* homepage */

	NULL,				/* load */
	NULL,				/* unload */
	NULL,				/* destroy */

	NULL,				/* ui_info */
	&prpl_info,			/* extra_info */
	NULL,				/* prefs_info */
	ggp_actions			/* actions */
};
/* }}} */

/* static void gaim_gg_debug_handler(int level, const char * format, va_list args) {{{ */
static void gaim_gg_debug_handler(int level, const char * format, va_list args) {
	GaimDebugLevel gaim_level;
	char *msg = g_strdup_vprintf(format, args);

	/* This is pretty pointless since the GG_DEBUG levels don't correspond to
	 * the gaim ones */
	switch (level) {
		case GG_DEBUG_FUNCTION:
			gaim_level = GAIM_DEBUG_INFO;
			break;
		case GG_DEBUG_MISC:
		case GG_DEBUG_NET:
		case GG_DEBUG_DUMP:
		case GG_DEBUG_TRAFFIC:
		default:
			gaim_level = GAIM_DEBUG_MISC;
			break;
	}

	gaim_debug(gaim_level, "gg", msg);
	g_free(msg);
}
/* }}} */

/*
 */
/* static void init_plugin(GaimPlugin *plugin) {{{ */
static void init_plugin(GaimPlugin *plugin)
{
	GaimAccountOption *option;

	option = gaim_account_option_string_new(_("Nickname"),
			"nick", _("Gadu-Gadu User"));
	prpl_info.protocol_options = g_list_append(prpl_info.protocol_options,
						   option);

	my_protocol = plugin;

	gg_debug_handler = gaim_gg_debug_handler;
}
/* }}} */

GAIM_INIT_PLUGIN(gg, init_plugin, info);

/* vim: set ts=8 sts=0 sw=8 noet: */
