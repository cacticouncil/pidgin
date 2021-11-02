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

#include <stdio.h>

#include "talkatu/talkatumenutoolbutton.h"

/**
 * SECTION:talkatumenutoolbutton
 * @Title: Menu Tool Button
 * @Short_description: A simple menu tool button.
 *
 * The normal #GtkMenuToolButton forces you to have an action on the always
 * visible button.  This #GtkToolItem instead just uses a label with no action.
 */

/**
 * TALKATU_TYPE_MENU_TOOL_BUTTON:
 *
 * The standard _get_type macro for #TalkatuMenuToolButton.
 */

/**
 * TalkatuMenuToolButton:
 *
 * A #GtkToolButton subclass that behaves like a #GtkComboBox.
 */

struct _TalkatuMenuToolButton {
	GtkToolButton parent;

	GtkWidget *menu;
};

G_DEFINE_TYPE(TalkatuMenuToolButton, talkatu_menu_tool_button, GTK_TYPE_TOOL_BUTTON)

enum {
	PROP_0 = 0,
	PROP_MENU,
	N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES] = {NULL,};

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/

static void
talkatu_menu_tool_button_clicked(GtkToolButton *button) {
	TalkatuMenuToolButton *menu_button = TALKATU_MENU_TOOL_BUTTON(button);

	gtk_menu_popup_at_widget(GTK_MENU(menu_button->menu),
	                         GTK_WIDGET(button),
	                         GDK_GRAVITY_SOUTH_WEST,
	                         GDK_GRAVITY_NORTH_WEST,
	                         NULL);
}

static void
talkatu_menu_tool_button_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec) {
	TalkatuMenuToolButton *menu_button = TALKATU_MENU_TOOL_BUTTON(obj);

	switch(prop_id) {
		case PROP_MENU:
			g_value_set_object(value, talkatu_menu_tool_button_get_menu(menu_button));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_menu_tool_button_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec) {
	TalkatuMenuToolButton *menu_button = TALKATU_MENU_TOOL_BUTTON(obj);

	switch(prop_id) {
		case PROP_MENU:
			talkatu_menu_tool_button_set_menu(menu_button, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_menu_tool_button_init(TalkatuMenuToolButton *menu_button) {
}

static void
talkatu_menu_tool_button_class_init(TalkatuMenuToolButtonClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkToolButtonClass *button_class = GTK_TOOL_BUTTON_CLASS(klass);

	obj_class->get_property = talkatu_menu_tool_button_get_property;
	obj_class->set_property = talkatu_menu_tool_button_set_property;

	button_class->clicked = talkatu_menu_tool_button_clicked;

	properties[PROP_MENU] = g_param_spec_object(
		"menu", "menu", "The menu to show",
		GTK_TYPE_MENU,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_menu_tool_button_new:
 * @label: The label to display.
 * @icon_name: The optional name of the icon to display.
 * @menu: The menu to display.
 *
 * Creates a new #TalkatuMenuToolButton with the given @label, @icon_name, and
 * @menu.
 *
 * Returns: (transfer full): The new #TalkatuMenuToolButton instance.
 */
GtkToolItem *
talkatu_menu_tool_button_new(const gchar *label, const gchar *icon_name, GtkWidget *menu) {
	return g_object_new(
		TALKATU_TYPE_MENU_TOOL_BUTTON,
		"label", label,
		"icon-name", icon_name,
		"menu", menu,
		NULL
	);
}

/**
 * talkatu_menu_tool_button_get_menu:
 * @menu_button: The #TalkatuMenuToolButton instance.
 *
 * Gets the menu that this tool button will display on click or #NULL if no
 * menu is set.
 *
 * Returns: (transfer full): The menu.
 */
GtkWidget *
talkatu_menu_tool_button_get_menu(TalkatuMenuToolButton *menu_button) {
	g_return_val_if_fail(TALKATU_IS_MENU_TOOL_BUTTON(menu_button), FALSE);

	if(menu_button->menu) {
		return g_object_ref(menu_button->menu);
	}

	return NULL;
}

/**
 * talkatu_menu_tool_button_set_menu:
 * @menu_button: The #TalkatuMenuToolButton instance.
 * @menu: The menu to set.
 *
 * Sets the menu to be displayed when the user clicks the button.
 */
void
talkatu_menu_tool_button_set_menu(TalkatuMenuToolButton *menu_button, GtkWidget *menu) {
	g_return_if_fail(TALKATU_IS_MENU_TOOL_BUTTON(menu_button));

	if(menu_button->menu) {
		gtk_menu_detach(GTK_MENU(menu_button->menu));
		g_object_unref(G_OBJECT(menu_button->menu));
	}

	if(menu) {
		menu_button->menu = GTK_WIDGET(g_object_ref(G_OBJECT(menu)));
		gtk_menu_attach_to_widget(GTK_MENU(menu_button->menu), GTK_WIDGET(menu_button), NULL);
	} else {
		menu_button->menu = NULL;
	}
}
