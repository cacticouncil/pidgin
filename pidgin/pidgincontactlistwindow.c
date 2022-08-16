/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "pidgincontactlistwindow.h"

#include "pidginactiongroup.h"

struct _PidginContactListWindow {
	GtkApplicationWindow parent;

	GtkWidget *vbox;
};

G_DEFINE_TYPE(PidginContactListWindow, pidgin_contact_list_window,
              GTK_TYPE_APPLICATION_WINDOW)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_contact_list_window_init(PidginContactListWindow *contact_list) {
	GSimpleActionGroup *group = NULL;

	gtk_widget_init_template(GTK_WIDGET(contact_list));

	gtk_window_set_application(GTK_WINDOW(contact_list),
	                           GTK_APPLICATION(g_application_get_default()));

	group = pidgin_action_group_new();
	gtk_widget_insert_action_group(GTK_WIDGET(contact_list), "blist",
	                               G_ACTION_GROUP(group));
}

static void
pidgin_contact_list_window_class_init(PidginContactListWindowClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/BuddyList/window.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginContactListWindow,
	                                     vbox);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_contact_list_window_new(void) {
	return g_object_new(PIDGIN_TYPE_CONTACT_LIST_WINDOW, NULL);
}

GtkWidget *
pidgin_contact_list_window_get_vbox(PidginContactListWindow *window) {
	g_return_val_if_fail(PIDGIN_IS_CONTACT_LIST_WINDOW(window), NULL);

	return window->vbox;
}
