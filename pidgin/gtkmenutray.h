/**
 * @file gtkmenutray.h GTK+ Tray menu item
 * @ingroup pidgin
 */

/* Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * under the terms of the GNU General Public License as published by
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
#ifndef PIDGIN_MENU_TRAY_H
#define PIDGIN_MENU_TRAY_H

#include <gtk/gtk.h>

#define PIDGIN_TYPE_MENU_TRAY            (pidgin_menu_tray_get_type())
#define PIDGIN_MENU_TRAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PIDGIN_TYPE_MENU_TRAY, PidginMenuTray))
#define PIDGIN_MENU_TRAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PIDGIN_TYPE_MENU_TRAY, PidginMenuTrayClass))
#define PIDGIN_IS_MENU_TRAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PIDGIN_TYPE_MENU_TRAY))
#define PIDGIN_IS_MENU_TRAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PIDGIN_TYPE_MENU_TRAY))
#define PIDGIN_MENU_TRAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PIDGIN_TYPE_MENU_TRAY, PidginMenuTrayClass))

typedef struct _PidginMenuTray				PidginMenuTray;
typedef struct _PidginMenuTrayClass		PidginMenuTrayClass;

/**
 * PidginMenuTray:
 * @tray: The tray
 *
 * A PidginMenuTray
 */
struct _PidginMenuTray {
	GtkMenuItem gparent;
	GtkWidget *tray;
};

struct _PidginMenuTrayClass {
	GtkMenuItemClass gparent;
};

G_BEGIN_DECLS

/**
 * pidgin_menu_tray_get_type:
 *
 * Registers the PidginMenuTray class if necessary and returns the
 * type ID assigned to it.
 *
 * Returns: The PidginMenuTray type ID
 */
GType pidgin_menu_tray_get_type(void);

/**
 * pidgin_menu_tray_new:
 *
 * Creates a new PidginMenuTray
 *
 * Returns: A new PidginMenuTray
 */
GtkWidget *pidgin_menu_tray_new(void);

/**
 * pidgin_menu_tray_get_box:
 * @menu_tray: The PidginMenuTray
 *
 * Gets the box for the PidginMenuTray
 *
 * Returns: The box that this menu tray is using
 */
GtkWidget *pidgin_menu_tray_get_box(PidginMenuTray *menu_tray);

/**
 * pidgin_menu_tray_append:
 * @menu_tray: The tray
 * @widget:    The widget
 * @tooltip:   The tooltip for this widget (widget requires its own X-window)
 *
 * Appends a widget into the tray
 */
void pidgin_menu_tray_append(PidginMenuTray *menu_tray, GtkWidget *widget, const char *tooltip);

/**
 * pidgin_menu_tray_prepend:
 * @menu_tray: The tray
 * @widget:    The widget
 * @tooltip:   The tooltip for this widget (widget requires its own X-window)
 *
 * Prepends a widget into the tray
 */
void pidgin_menu_tray_prepend(PidginMenuTray *menu_tray, GtkWidget *widget, const char *tooltip);

/**
 * pidgin_menu_tray_set_tooltip:
 * @menu_tray: The tray
 * @widget:    The widget
 * @tooltip:   The tooltip to set for the widget (widget requires its own X-window)
 *
 * Set the tooltip for a widget
 */
void pidgin_menu_tray_set_tooltip(PidginMenuTray *menu_tray, GtkWidget *widget, const char *tooltip);

G_END_DECLS

#endif /* PIDGIN_MENU_TRAY_H */
