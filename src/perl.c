/*
 * gaim
 *
 * Copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
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
 *
 * This was taken almost exactly from X-Chat. The power of the GPL.
 * Translated from X-Chat to Gaim by Eric Warmenhoven.
 * Originally by Erik Scrafford <eriks@chilisoft.com>.
 * X-Chat Copyright (C) 1998 Peter Zelezny.
 *
 */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#undef PACKAGE

#ifdef USE_PERL

#include <EXTERN.h>
#ifndef _SEM_SEMUN_UNDEFINED
#define HAS_UNION_SEMUN
#endif
#include <perl.h>
#include <XSUB.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#undef PACKAGE
#include <stdio.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include "pixmaps/add.xpm"
#include "pixmaps/cancel.xpm"


/* perl module support */
extern void xs_init _((void));
extern void boot_DynaLoader _((CV * cv)); /* perl is so wacky */

#undef _
#include "gaim.h"

struct perlscript {
	char *name;
	char *version;
	char *shutdowncallback; /* bleh */
};

struct _perl_event_handlers {
	char *event_type;
	char *handler_name;
};

struct _perl_timeout_handlers {
	char *handler_name;
	gint iotag;
};

static GList *perl_list = NULL; /* should probably extern this at some point */
static GList *perl_timeout_handlers = NULL;
static GList *perl_event_handlers = NULL;
static PerlInterpreter *my_perl = NULL;

/* dealing with gaim */
XS(XS_AIM_register); /* set up hooks for script */
XS(XS_AIM_get_info); /* version, last to attempt signon, protocol */
XS(XS_AIM_print); /* lemme figure this one out... */

/* list stuff */
XS(XS_AIM_buddy_list); /* all buddies */
XS(XS_AIM_online_list); /* online buddies */
XS(XS_AIM_deny_list); /* also returns permit list */

/* server stuff */
XS(XS_AIM_command); /* send command to server */
XS(XS_AIM_user_info); /* given name, return struct buddy members */
XS(XS_AIM_print_to_conv); /* send message to someone */
XS(XS_AIM_print_to_chat); /* send message to chat room */

/* handler commands */
XS(XS_AIM_add_event_handler); /* when servers talk */
XS(XS_AIM_add_timeout_handler); /* figure it out */

void xs_init()
{
	char *file = __FILE__;
	newXS ("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}

static char *escape_quotes(char *buf)
{
	static char *tmp_buf = NULL;
	char *i, *j;

	if (tmp_buf)
		g_free(tmp_buf);
	tmp_buf = g_malloc(strlen(buf) * 2 + 1);
	for (i = buf, j = tmp_buf; *i; i++, j++) {
		if (*i == '\'' || *i == '\\')
			*j++ = '\\';
		*j = *i;
	}
	*j = '\0';

	return (tmp_buf);
}

static SV *execute_perl(char *function, char *args)
{
	static char *perl_cmd = NULL;

	if (perl_cmd)
		g_free(perl_cmd);
	perl_cmd = g_malloc(strlen(function) + strlen(args) + 2 + 10);
	sprintf(perl_cmd, "&%s('%s')", function, escape_quotes(args));
#ifndef HAVE_PERL_EVAL_PV
	return (perl_eval_pv(perl_cmd, TRUE));
#else
	return (Perl_eval_pv(perl_cmd, TRUE));
#endif
}

int perl_load_file(char *script_name)
{
	SV *return_val;
	return_val = execute_perl("load_file", script_name);
	return SvNV (return_val);
}

static int is_pl_file(char *filename)
{
	int len;
	if (!filename) return 0;
	if (!filename[0]) return 0;
	len = strlen(filename);
	len -= 3;
	if (len < 0) return 0;
	return (!strncmp(filename + len, ".pl", 3));
}

void perl_autoload()
{
	DIR *dir;
	struct dirent *ent;
	char *buf;
	char path[BUF_LONG];

	g_snprintf(path, sizeof(path), "%s/.gaim", getenv("HOME"));
	dir = opendir(path);
	if (dir) {
		while ((ent = readdir(dir))) {
			if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
				if (is_pl_file(ent->d_name)) {
					buf = g_malloc(strlen(path) + strlen(ent->d_name) + 2);
					sprintf(buf, "%s/%s", path, ent->d_name);
					perl_load_file(buf);
					g_free(buf);
				}
			}
		}
		closedir(dir);
	}
}

void perl_init()
{
	char *perl_args[] = {"", "-e", "0", "-w"};
	char load_file[] =
"sub load_file()\n"
"{\n"
"	(my $file_name) = @_;\n"
"	open FH, $file_name or return 2;\n"
"	my $is = $/;\n"
"	local($/) = undef;\n"
"	$file = <FH>;\n"
"	close FH;\n"
"	$/ = $is;\n"
"	$file = \"\\@ISA = qw(Exporter DynaLoader);\\n\" . $file;\n"
"	eval $file;\n"
"	eval $file if $@;\n"
"	return 1 if $@;\n"
"	return 0;\n"
"}";

	my_perl = perl_alloc();
	perl_construct(my_perl);
	perl_parse(my_perl, xs_init, 4, perl_args, NULL);
#ifndef HAVE_PERL_EVAL_PV
	perl_eval_pv(load_file, TRUE);
#else
	Perl_eval_pv(load_file, TRUE);
#endif

	newXS ("AIM::register", XS_AIM_register, "AIM");
	newXS ("AIM::get_info", XS_AIM_get_info, "AIM");
	newXS ("AIM::print", XS_AIM_print, "AIM");

	newXS ("AIM::buddy_list", XS_AIM_buddy_list, "AIM");
	newXS ("AIM::online_list", XS_AIM_online_list, "AIM");
	newXS ("AIM::deny_list", XS_AIM_deny_list, "AIM");

	newXS ("AIM::command", XS_AIM_command, "AIM");
	newXS ("AIM::user_info", XS_AIM_user_info, "AIM");
	newXS ("AIM::print_to_conv", XS_AIM_print_to_conv, "AIM");
	newXS ("AIM::print_to_chat", XS_AIM_print_to_chat, "AIM");

	newXS ("AIM::add_event_handler", XS_AIM_add_event_handler, "AIM");
	newXS ("AIM::add_timeout_handler", XS_AIM_add_timeout_handler, "AIM");
}

void perl_end()
{
	struct perlscript *scp;
	struct _perl_timeout_handlers *thn;
	struct _perl_event_handlers *ehn;

	while (perl_list) {
		scp = perl_list->data;
		perl_list = g_list_remove(perl_list, scp);
		if (scp->shutdowncallback[0])
			execute_perl(scp->shutdowncallback, "");
		g_free(scp->name);
		g_free(scp->version);
		g_free(scp->shutdowncallback);
		g_free(scp);
	}

	while (perl_timeout_handlers) {
		thn = perl_timeout_handlers->data;
		perl_timeout_handlers = g_list_remove(perl_timeout_handlers, thn);
		gtk_timeout_remove(thn->iotag);
		g_free(thn->handler_name);
		g_free(thn);
	}

	while (perl_event_handlers) {
		ehn = perl_event_handlers->data;
		perl_event_handlers = g_list_remove(perl_event_handlers, ehn);
		g_free(ehn->event_type);
		g_free(ehn->handler_name);
		g_free(ehn);
	}

	if (my_perl != NULL) {
		perl_destruct(my_perl);
		perl_free(my_perl);
		my_perl = NULL;
	}
}

XS (XS_AIM_register)
{
	char *name, *ver, *callback, *unused; /* exactly like X-Chat, eh? :) */
	int junk;
	struct perlscript *scp;
	dXSARGS;
	items = 0;

	name = SvPV (ST (0), junk);
	ver = SvPV (ST (1), junk);
	callback = SvPV (ST (2), junk);
	unused = SvPV (ST (3), junk);

	scp = g_new0(struct perlscript, 1);
	scp->name = g_strdup(name);
	scp->version = g_strdup(ver);
	scp->shutdowncallback = g_strdup(callback);
	perl_list = g_list_append(perl_list, scp);

	XST_mPV (0, VERSION);
	XSRETURN (1);
}

XS (XS_AIM_get_info)
{
	int junk;
	dXSARGS;
	items = 0;

	switch(atoi(SvPV(ST(0), junk))) {
	case 0:
		XST_mPV(0, VERSION);
		break;
	case 1:
		/* FIXME: no more current_user
		XST_mPV(0, current_user->username);
		*/
		break;
	case 2:
		/* FIXME: more per-connection issues
		if (!blist)
			XST_mPV(0, "Offline");
		else if (!USE_OSCAR)
			XST_mPV(0, "TOC");
		else
			XST_mPV(0, "Oscar");
		*/
		break;
	default:
		XST_mPV(0, "Error2");
	}

	XSRETURN(1);
}

XS (XS_AIM_print)
{
	char *title;
	char *message;
	int junk;
	dXSARGS;
	items = 0;

	title = SvPV(ST(0), junk);
	message = SvPV(ST(1), junk);
	do_error_dialog(message, title);
	XSRETURN(0);
}

XS (XS_AIM_buddy_list)
{
	struct buddy *buddy;
	struct group *g;
	GSList *list = groups;
	GList *mem;
	int i = 0;
	dXSARGS;
	items = 0;

	while (list) {
		g = (struct group *)list->data;
		mem = g->members;
		while (mem) {
			buddy = (struct buddy *)mem->data;
			XST_mPV(i++, buddy->name);
			mem = mem->next;
		}
		list = g_slist_next(list);
	}
	XSRETURN(i);
}

XS (XS_AIM_online_list)
{
	struct buddy *b;
	struct group *g;
	GSList *list = groups;
	GList *mem;
	int i = 0;
	dXSARGS;
	items = 0;

	while (list) {
		g = (struct group *)list->data;
		mem = g->members;
		while (mem) {
			b = (struct buddy *)mem->data;
			if (b->present) XST_mPV(i++, b->name);
			mem = mem->next;
		}
		list = g_slist_next(list);
	}
	XSRETURN(i);
}

XS (XS_AIM_deny_list)
{
	char *name;
	GList *list = deny;
	int i = 0;
	dXSARGS;
	items = 0;

	while (list) {
		name = (char *)list->data;
		XST_mPV(i++, name);
		list = list->next;
	}
	XSRETURN(i);
}

XS (XS_AIM_command)
{
	int junk;
	char *command = NULL;
	dXSARGS;
	items = 0;

	command = SvPV(ST(0), junk);
	if (!command) XSRETURN(0);
	if        (!strncasecmp(command, "signon", 6)) {
		/* FIXME
		if (!blist) {
			show_login();
			dologin(0, 0);
		}
		*/
	} else if (!strncasecmp(command, "signoff", 7)) {
		/* FIXME: we need to figure out how this works for multiple connections
		 * signoff(); */
	} else if (!strncasecmp(command, "away", 4)) {
		char *message = SvPV(ST(1), junk);
		struct away_message a;
		g_snprintf(a.message, sizeof(a.message), "%s", message);
		do_away_message(NULL, &a);
	} else if (!strncasecmp(command, "back", 4)) {
		do_im_back();
	} else if (!strncasecmp(command, "idle", 4)) {
		/* FIXME
		serv_set_idle(atoi(SvPV(ST(1), junk)));
		*/
	} else if (!strncasecmp(command, "warn", 4)) {
		/* yet another perl FIXME
		char *name = SvPV(ST(1), junk);
		serv_warn(name, 0);
		*/
	}

	XSRETURN(0);
}

XS (XS_AIM_user_info)
{
	int junk;
	struct buddy *buddy;
	char *nick;
	dXSARGS;
	items = 0;

	nick = SvPV(ST(0), junk);
	if (!nick[0])
		XSRETURN(0);
	buddy = find_buddy(nick);
	if (!buddy)
		XSRETURN(0);
	XST_mPV(0, buddy->name);
	XST_mPV(1, buddy->present ? "Online" : "Offline");
	XST_mIV(2, buddy->evil);
	XST_mIV(3, buddy->signon);
	XST_mIV(4, buddy->idle);
	XST_mIV(5, buddy->uc);
	XST_mIV(6, buddy->caps);
	XSRETURN(7);
}

XS (XS_AIM_print_to_conv)
{
	char *nick, *what;
	struct conversation *c;
	int junk;
	dXSARGS;
	items = 0;

	nick = SvPV(ST(0), junk);
	what = SvPV(ST(1), junk);
	c = find_conversation(nick);
	if (!c)
		c = new_conversation(nick);
	write_to_conv(c, what, WFLAG_SEND, NULL);
	serv_send_im(nick, what, 0);
}

XS (XS_AIM_print_to_chat)
{
	/* FIXME: need to make this multi-connection based
	char *nick, *what;
	struct conversation *c = NULL;
	GList *bcs = buddy_chats;
	int junk;
	dXSARGS;
	items = 0;

	nick = SvPV(ST(0), junk);
	what = SvPV(ST(1), junk);
	while (bcs) {
		c = (struct conversation *)bcs->data;
		if (!strcmp(c->name, nick))
			break;
		bcs = bcs->next;
		c = NULL;
	}
	if (!c)
		XSRETURN(0);
	serv_chat_send(c->id, what);
	*/
}

int perl_event(char *event, char *args)
{
	GList *handler;
	struct _perl_event_handlers *data;

	for (handler = perl_event_handlers; handler != NULL; handler = handler->next) {
		data = handler->data;
		if (!strcmp(event, data->event_type))
			execute_perl(data->handler_name, args);
	}

	return 0;
}

XS (XS_AIM_add_event_handler)
{
	int junk;
	struct _perl_event_handlers *handler;
	dXSARGS;
	items = 0;

	handler = g_new0(struct _perl_event_handlers, 1);
	handler->event_type = g_strdup(SvPV(ST(0), junk));
	handler->handler_name = g_strdup(SvPV(ST(1), junk));
	perl_event_handlers = g_list_append(perl_event_handlers, handler);
	sprintf(debug_buff, "registered perl event handler for %s\n", handler->event_type);
	debug_print(debug_buff);
	XSRETURN_EMPTY;
}

static int perl_timeout(struct _perl_timeout_handlers *handler)
{
	execute_perl(handler->handler_name, "");
	perl_timeout_handlers = g_list_remove(perl_timeout_handlers, handler);
	g_free(handler->handler_name);
	g_free(handler);

	return 0; /* returning zero removes the timeout handler */
}

XS (XS_AIM_add_timeout_handler)
{
	int junk;
	long timeout;
	struct _perl_timeout_handlers *handler;
	dXSARGS;
	items = 0;

	handler = g_new0(struct _perl_timeout_handlers, 1);
	timeout = 1000 * atol(SvPV(ST(0), junk));
	handler->handler_name = g_strdup(SvPV(ST(1), junk));
	perl_timeout_handlers = g_list_append(perl_timeout_handlers, handler);
	handler->iotag = gtk_timeout_add(timeout, (GtkFunction)perl_timeout, handler);
	XSRETURN_EMPTY;
}

static GtkWidget *config = NULL;
static GtkWidget *entry = NULL;

static void cfdes(GtkWidget *m, gpointer n) {
	if (config) gtk_widget_destroy(config);
	config = NULL;
}

static void do_load(GtkWidget *m, gpointer n) {
	char *file = gtk_entry_get_text(GTK_ENTRY(entry));
	if (!file || !strlen(file)) {
		perl_end();
		perl_init();
		return;
	}
	perl_load_file(file);
	gtk_widget_destroy(config);
}

void load_perl_script(GtkWidget *w, gpointer d)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *ok;
	GtkWidget *cancel;

	if (config) {
		gtk_widget_show(config);
		return;
	}

	config = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_policy(GTK_WINDOW(config), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(config), "perl_script", "Gaim");
	gtk_window_set_title(GTK_WINDOW(config), "Gaim - Add Perl Script");
	gtk_container_set_border_width(GTK_CONTAINER(config), 5);
	gtk_signal_connect(GTK_OBJECT(config), "destroy", GTK_SIGNAL_FUNC(cfdes), 0);
	gtk_widget_realize(config);
	aol_icon(config->window);

	frame = gtk_frame_new("Load Script");
	gtk_container_add(GTK_CONTAINER(config), frame);
	gtk_widget_show(frame);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 5);
	gtk_widget_show(hbox);

	label = gtk_label_new("File Name:");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(entry), "activate", GTK_SIGNAL_FUNC(do_load), 0);
	gtk_widget_show(entry);

	hbox = gtk_hbox_new(TRUE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 5);
	gtk_widget_show(hbox);

	ok = picture_button(config, "Load", add_xpm);
	gtk_box_pack_start(GTK_BOX(hbox), ok, FALSE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked", GTK_SIGNAL_FUNC(do_load), 0);

	cancel = picture_button(config, "Cancel", cancel_xpm);
	gtk_box_pack_start(GTK_BOX(hbox), cancel, FALSE, FALSE, 5);
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked", GTK_SIGNAL_FUNC(cfdes), 0);

	gtk_widget_show(config);
}

extern void unload_perl_scripts(GtkWidget *w, gpointer d)
{
	perl_end();
	perl_init();
}

extern void list_perl_scripts(GtkWidget *w, gpointer d)
{
	GList *s = perl_list;
	struct perlscript *p;
	char buf[BUF_LONG * 4];
	int at = 0;

	at += g_snprintf(buf + at, sizeof(buf) - at, "Loaded scripts:\n");
	while (s) {
		p = (struct perlscript *)s->data;
		at += g_snprintf(buf + at, sizeof(buf) - at, "%s\n", p->name);
		s = s->next;
	}

	do_error_dialog(buf, _("Perl Scripts"));
}

#endif /* USE_PERL */
