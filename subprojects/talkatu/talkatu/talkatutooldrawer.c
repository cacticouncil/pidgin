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

#include "talkatu/talkatutooldrawer.h"
#include "talkatu/talkatumenutoolbutton.h"

/**
 * SECTION:talkatutooldrawer
 * @Title: ToolDrawer Tool Item
 * @Short_description: A GtkToolItem that can be expand to show multiple items.
 *
 * A #TalkatuToolDrawer is a #GtkToolItem that groups many #GtkToolItems into a
 * collection that can be collapsed into a single item to eliminate clutter in
 * the user interface.
 */

/**
 * TALKATU_TYPE_TOOL_DRAWER:
 *
 * The standard _get_type macro for #TalkatuToolDrawer.
 */

typedef struct {
	GAction *action;
	gchar *markup;
	gchar *icon_name;
	gchar *tooltip;
	GCallback callback;
} TalkatuToolDrawerItem;

/**
 * TalkatuToolDrawer:
 *
 * A #GtkToolItem subclass that displays a menu of items or can be expanded
 * to show all of the items as a toolbar.
 *
 * Stability: Unstable
 */
struct _TalkatuToolDrawer {
	GtkToolItem parent;

	gchar *label;
	gchar *icon_name;
	gboolean expanded;

	GtkWidget *menu;

	GtkToolItem *menu_button;
	GtkWidget *box;
	GtkWidget *icons;
};

enum {
	PROP_0 = 0,
	PROP_LABEL,
	PROP_ICON_NAME,
	PROP_EXPANDED,
	N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE(TalkatuToolDrawer, talkatu_tool_drawer, GTK_TYPE_TOOL_ITEM)

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_tool_drawer_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec) {
	TalkatuToolDrawer *drawer = TALKATU_TOOL_DRAWER(obj);

	switch(prop_id) {
		case PROP_LABEL:
			g_value_set_string(value, talkatu_tool_drawer_get_label(drawer));
			break;
		case PROP_ICON_NAME:
			g_value_set_string(value, talkatu_tool_drawer_get_icon_name(drawer));
			break;
		case PROP_EXPANDED:
			g_value_set_boolean(value, talkatu_tool_drawer_get_expanded(drawer));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_tool_drawer_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec) {
	TalkatuToolDrawer *drawer = TALKATU_TOOL_DRAWER(obj);

	switch(prop_id) {
		case PROP_LABEL:
			talkatu_tool_drawer_set_label(drawer, g_value_get_string(value));
			break;
		case PROP_ICON_NAME:
			talkatu_tool_drawer_set_icon_name(drawer, g_value_get_string(value));
			break;
		case PROP_EXPANDED:
			talkatu_tool_drawer_set_expanded(drawer, g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
talkatu_tool_drawer_init(TalkatuToolDrawer *drawer) {
}

static void
talkatu_tool_drawer_constructed(GObject *obj) {
	TalkatuToolDrawer *drawer = TALKATU_TOOL_DRAWER(obj);

	drawer->menu = gtk_menu_new();

	drawer->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(drawer), drawer->box);

	drawer->menu_button = talkatu_menu_tool_button_new(drawer->label, drawer->icon_name, drawer->menu);
	gtk_box_pack_start(GTK_BOX(drawer->box), GTK_WIDGET(drawer->menu_button), FALSE, FALSE, 0);

	drawer->icons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_pack_start(GTK_BOX(drawer->box), drawer->icons, FALSE, FALSE, 0);

	/* Start collapsed. */
	g_object_set(G_OBJECT(drawer->menu_button), "no-show-all", FALSE, NULL);
	gtk_widget_show_all(GTK_WIDGET(drawer->menu_button));
	g_object_set(G_OBJECT(drawer->icons), "no-show-all", TRUE, NULL);
}

static void
talkatu_tool_drawer_class_init(TalkatuToolDrawerClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = talkatu_tool_drawer_get_property;
	obj_class->set_property = talkatu_tool_drawer_set_property;
	obj_class->constructed = talkatu_tool_drawer_constructed;

	properties[PROP_LABEL] = g_param_spec_string(
		"label", "label", "The label to display when not expanded",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);
	properties[PROP_ICON_NAME] = g_param_spec_string(
		"icon-name", "icon-name", "The name of the icon to display when not expanded",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
	);
	properties[PROP_EXPANDED] = g_param_spec_boolean(
		"expanded", "expanded", "Whether or not the drawer is expanded",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_tool_drawer_new:
 * @label: The label to give item.
 * @icon_name: The name of the icon for this item.
 *
 * Creates a new #TalkatuToolDrawer instance.
 *
 * Returns: (transfer full): The new #TalkatuToolDrawer instance.
 */
GtkToolItem *
talkatu_tool_drawer_new(const gchar *label, const gchar *icon_name) {
	return g_object_new(
		TALKATU_TYPE_TOOL_DRAWER,
		"label", label,
		"icon-name", icon_name,
		NULL
	);
}

/**
 * talkatu_tool_drawer_add_item:
 * @drawer: The #TalkatuToolDrawer instance.
 * @action: The #GAction to add.
 * @markup: Pango markup to use as a label.
 * @icon_name: The name of the icon to display.
 * @tooltip: UTF-8 text to display as a tooltip.
 * @callback: (scope notified): The callback to call when the item is
 *                              activated.
 *
 * Adds a new item to @drawer.
 */
void
talkatu_tool_drawer_add_item(TalkatuToolDrawer *drawer, GAction *action, const gchar *markup, const gchar *icon_name, gchar *tooltip, GCallback callback) {
	GtkWidget *button = NULL, *item = NULL, *label = NULL, *image = NULL;
	const GVariantType *state_type = NULL;

	g_return_if_fail(TALKATU_IS_TOOL_DRAWER(drawer));

	/* create the menu item */
	state_type = g_action_get_state_type(action);
	if(state_type) {
		item = gtk_check_menu_item_new();
	} else {
		item = gtk_menu_item_new();
	}

	label = gtk_label_new(markup);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_container_add(GTK_CONTAINER(item), label);

	gtk_menu_shell_append(GTK_MENU_SHELL(drawer->menu), item);
	gtk_widget_show_all(item);

	/* create the toolbar button and add it to the toolbar */
	if(state_type) {
		button = gtk_toggle_button_new();
	} else {
		button = gtk_button_new();
	}

	gtk_widget_set_can_focus(button, FALSE);
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

	image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(button), image);

	gtk_box_pack_start(GTK_BOX(drawer->icons), button, FALSE, FALSE, 0);
}

/**
 * talkatu_tool_drawer_add_separator:
 * @drawer: The #TalkatuToolDrawer instance.
 *
 * Adds a separator to the end of @drawer.
 */
void
talkatu_tool_drawer_add_separator(TalkatuToolDrawer *drawer) {
	g_return_if_fail(TALKATU_IS_TOOL_DRAWER(drawer));

	gtk_menu_shell_append(GTK_MENU_SHELL(drawer->menu), gtk_separator_menu_item_new());

	gtk_box_pack_start(GTK_BOX(drawer->icons), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);
}

/**
 * talkatu_tool_drawer_get_label:
 * @drawer: The #TalkatuToolDrawer instance.
 *
 * Gets the label that's displayed when @drawer is collapsed.
 *
 * Returns: The label to use when @drawer is collapsed.
 */
const gchar *
talkatu_tool_drawer_get_label(TalkatuToolDrawer *drawer) {
	g_return_val_if_fail(TALKATU_IS_TOOL_DRAWER(drawer), FALSE);

	return drawer->label;
}

/**
 * talkatu_tool_drawer_set_label:
 * @drawer: The #TalkatuToolDrawer instance.
 * @label: The label to use when @drawer is collapsed.
 *
 * Sets the label to be displayed when @drawer is collapsed.
 */
void
talkatu_tool_drawer_set_label(TalkatuToolDrawer *drawer, const gchar *label) {
	g_return_if_fail(TALKATU_IS_TOOL_DRAWER(drawer));

	g_free(drawer->label);

	drawer->label = g_strdup(label);

	g_object_notify_by_pspec(G_OBJECT(drawer), properties[PROP_LABEL]);
}

/**
 * talkatu_tool_drawer_get_icon_name:
 * @drawer: The #TalkatuToolDrawer instance.
 *
 * Gets the icon name for @drawer.
 *
 * Returns: The icon name for @drawer.
 */
const gchar *
talkatu_tool_drawer_get_icon_name(TalkatuToolDrawer *drawer) {
	g_return_val_if_fail(TALKATU_IS_TOOL_DRAWER(drawer), FALSE);

	return drawer->icon_name;
}

/**
 * talkatu_tool_drawer_set_icon_name:
 * @drawer: The #TalkatuToolDrawer instance.
 * @icon_name: The name of the icon to display when collapse.
 *
 * Sets the name of the icon to be displayed when @drawer is collapsed.
 */
void
talkatu_tool_drawer_set_icon_name(TalkatuToolDrawer *drawer, const gchar *icon_name) {
	g_return_if_fail(TALKATU_IS_TOOL_DRAWER(drawer));

	g_free(drawer->icon_name);

	drawer->icon_name = g_strdup(icon_name);

	g_object_notify_by_pspec(G_OBJECT(drawer), properties[PROP_ICON_NAME]);
}

/**
 * talkatu_tool_drawer_set_expanded:
 * @drawer: The #TalkatuToolDrawer instance.
 * @expanded: %TRUE to expand @drawer, %FALSE to collapse.
 *
 * Sets whether or not @drawer is expanded.
 */
void
talkatu_tool_drawer_set_expanded(TalkatuToolDrawer *drawer, gboolean expanded) {
	g_return_if_fail(TALKATU_IS_TOOL_DRAWER(drawer));

	if(expanded != drawer->expanded) {
		drawer->expanded = expanded;

		g_object_set(G_OBJECT(drawer->menu_button), "no-show-all", drawer->expanded, NULL);
		g_object_set(G_OBJECT(drawer->icons), "no-show-all", !drawer->expanded, NULL);
		if(expanded) {
			gtk_widget_hide(GTK_WIDGET(drawer->menu_button));
			gtk_widget_show_all(drawer->icons);
		} else {
			gtk_widget_show_all(GTK_WIDGET(drawer->menu_button));
			gtk_widget_hide(drawer->icons);
		}

		g_object_notify_by_pspec(G_OBJECT(drawer), properties[PROP_EXPANDED]);
	}
}

/**
 * talkatu_tool_drawer_get_expanded:
 * @drawer: The #TalkatuToolDrawer instance.
 *
 * Gets whether or not @drawer is expanded.
 *
 * Returns: %TRUE if @drawer is expanded, %FALSE otherwise.
 */
gboolean
talkatu_tool_drawer_get_expanded(TalkatuToolDrawer *drawer) {
	g_return_val_if_fail(TALKATU_IS_TOOL_DRAWER(drawer), FALSE);

	return drawer->expanded;
}
