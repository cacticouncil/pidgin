/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNT_VERSION_H
#define GNT_VERSION_H
/**
 * SECTION:gntversion
 * @section_id: libgnt-gntversion
 * @title: Version Information
 * @short_description: Variables to check the GNT version
 */

/**
 * GNT_VERSION:
 *
 * The full version number. For example 2.14.0.
 */
#define GNT_VERSION "3.0.0-devel"

/**
 * GNT_MAJOR_VERSION:
 *
 * The major version number. For example, if the version is "2.14.0devel", this
 * is the "2".
 */
#define GNT_MAJOR_VERSION 3

/**
 * GNT_MINOR_VERSION:
 *
 * The minor version number. For example, if the version is "2.14.0devel", this
 * is the "14".
 */
#define GNT_MINOR_VERSION 0

/**
 * GNT_MICRO_VERSION:
 *
 * The micro version number. For example, if the version is "2.14.0devel", this
 * is the "0".
 */
#define GNT_MICRO_VERSION 0

/**
 * GNT_EXTRA_VERSION:
 *
 * The extra part of the version. For example, if the version is "2.14.0devel"
 * this is "devel".
 */
#define GNT_EXTRA_VERSION "devel"

#endif /* GNT_VERSION_H */
