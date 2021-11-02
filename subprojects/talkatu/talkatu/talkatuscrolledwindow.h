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

#ifndef TALKATU_SCROLLED_WINDOW_H
#define TALKATU_SCROLLED_WINDOW_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_SCROLLED_WINDOW (talkatu_scrolled_window_get_type())

G_DECLARE_FINAL_TYPE(TalkatuScrolledWindow, talkatu_scrolled_window, TALKATU, SCROLLED_WINDOW, GtkScrolledWindow)

GtkWidget *talkatu_scrolled_window_new(GtkAdjustment *hadjustment, GtkAdjustment *vadjustment);

void talkatu_scrolled_window_page_up(TalkatuScrolledWindow *sw);
void talkatu_scrolled_window_page_down(TalkatuScrolledWindow *sw);

G_END_DECLS

#endif /* TALKATU_SCROLLED_WINDOW_H */
