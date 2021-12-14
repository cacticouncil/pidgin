/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PURPLE_SOUPCOMPAT_H
#define PURPLE_SOUPCOMPAT_H
/*
 * This file is internal to libpurple. Do not use!
 * Also, any public API should not depend on this file.
 */

#include <libsoup/soup.h>

#if SOUP_MAJOR_VERSION < 3

static inline const char *
soup_message_get_reason_phrase(SoupMessage *msg) {
	return msg->reason_phrase;
}

static inline SoupMessageHeaders *
soup_message_get_request_headers(SoupMessage *msg) {
	return msg->request_headers;
}

static inline SoupStatus
soup_message_get_status(SoupMessage *msg) {
	return msg->status_code;
}

#endif /* SOUP_MAJOR_VERSION < 3 */

#endif /* PURPLE_SOUPCOMPAT_H */
