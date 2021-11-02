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

#ifndef GNT_H
#define GNT_H

#include <glib.h>

#ifndef __GI_SCANNER__
# ifdef GNT_COMPILATION
#  error "gnt source files should not include gnt.h"
# endif /* GNT_COMPILATION */
#endif /* __GI_SCANNER__ */

#ifndef GNT_GLOBAL_HEADER_INSIDE
# define GNT_GLOBAL_HEADER_INSIDE
#endif /* GNT_GLOBAL_HEADER_INSIDE */

#include <gntbindable.h>
#include <gntbox.h>
#include <gntbutton.h>
#include <gntcheckbox.h>
#include <gntclipboard.h>
#include <gntcolors.h>
#include <gntcombobox.h>
#include <gntentry.h>
#include <gntfilesel.h>
#include <gntkeys.h>
#include <gntlabel.h>
#include <gntline.h>
#include <gntmain.h>
#include <gntmenu.h>
#include <gntmenuitem.h>
#include <gntmenuitemcheck.h>
#include <gntprogressbar.h>
#include <gntslider.h>
#include <gntstyle.h>
#include <gnttextview.h>
#include <gnttree.h>
#include <gntutils.h>
#include <gntwidget.h>
#include <gntwindow.h>
#include <gntwm.h>
#include <gntws.h>
#include <gntncurses.h>
#include <gntversion.h>

#undef GNT_GLOBAL_HEADER_INSIDE

#endif /* GNT_H */
