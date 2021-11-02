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

#if !defined(TALKATU_GLOBAL_HEADER_INSIDE) && !defined(TALKATU_COMPILATION)
#error "only <talkatu.h> may be included directly"
#endif

#ifndef TALKATU_TOOL_DRAWER_H
#define TALKATU_TOOL_DRAWER_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_TOOL_DRAWER            (talkatu_tool_drawer_get_type())

G_DECLARE_FINAL_TYPE(TalkatuToolDrawer, talkatu_tool_drawer, TALKATU, TOOL_DRAWER, GtkToolItem)

GtkToolItem *talkatu_tool_drawer_new(const gchar *label, const gchar *icon_name);

void talkatu_tool_drawer_add_item(TalkatuToolDrawer *drawer, GAction *action, const gchar *markup, const gchar *icon_name, gchar *tooltip, GCallback callback);
void talkatu_tool_drawer_add_separator(TalkatuToolDrawer *drawer);

const gchar *talkatu_tool_drawer_get_label(TalkatuToolDrawer *drawer);
void talkatu_tool_drawer_set_label(TalkatuToolDrawer *drawer, const gchar *label);

const gchar *talkatu_tool_drawer_get_icon_name(TalkatuToolDrawer *drawer);
void talkatu_tool_drawer_set_icon_name(TalkatuToolDrawer *drawer, const gchar *icon_name);

void talkatu_tool_drawer_set_expanded(TalkatuToolDrawer *drawer, gboolean expanded);
gboolean talkatu_tool_drawer_get_expanded(TalkatuToolDrawer *drawer);

G_END_DECLS

#endif /* TALKATU_TOOL_DRAWER_H */
