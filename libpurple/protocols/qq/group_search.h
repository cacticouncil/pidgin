/**
 * @file group_search.h
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#ifndef _QQ_GROUP_SEARCH_H_
#define _QQ_GROUP_SEARCH_H_

#include <glib.h>
#include "connection.h"

enum {
	QQ_ROOM_SEARCH_ONLY = 0,
	QQ_ROOM_SEARCH_FOR_JOIN
};

void qq_request_room_search(PurpleConnection *gc, guint32 ext_id, int action);
void qq_process_room_search(PurpleConnection *gc, guint8 *data, gint len, guint32 ship32);

#endif
