/*
 * pidgin
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
 * along with this program; if not, it can be viewed online at
 * https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 */

#ifndef PIDGIN_PIDGIN_H
#define PIDGIN_PIDGIN_H

/**
 * SECTION:pidgin
 * @section_id: libpurple-purple
 * @short_description: <filename>pidgin.h</filename>
 * @title: Global header for Pidgin
 *
 * This file contains all the necessary preprocessor directives to include
 * Pidgin's headers and other preprocessor directives required for plugins
 * to build.  Including this file eliminates the need to directly include any
 * other Pidgin files.
 */

#include <glib.h>

#ifndef __GI_SCANNER__
# ifdef PIDGIN_COMPILATION
#  error "pidgin source files should not be including pidgin.h"
# endif /* PIDGIN_COMPILATION */
#endif /* __GI_SCANNER__ */

#ifndef PIDGIN_GLOBAL_HEADER_INSIDE
# define PIDGIN_GLOBAL_HEADER_INSIDE
#endif /* PIDGIN_GLOBAL_HEADER_INSIDE */

#include <pidgin/gtkaccount.h>
#include <pidgin/gtkblist.h>
#include <pidgin/gtkconn.h>
#include <pidgin/gtkconv.h>
#include <pidgin/gtkconvwin.h>
#include <pidgin/gtkdialogs.h>
#include <pidgin/gtkdnd-hints.h>
#include <pidgin/gtkicon-theme.h>
#include <pidgin/gtkicon-theme-loader.h>
#include <pidgin/gtkidle.h>
#include <pidgin/gtkmedia.h>
#include <pidgin/gtknotify.h>
#include <pidgin/gtkpluginpref.h>
#include <pidgin/gtkprefs.h>
#include <pidgin/gtkprivacy.h>
#include <pidgin/gtkrequest.h>
#include <pidgin/gtkroomlist.h>
#include <pidgin/gtksavedstatuses.h>
#include <pidgin/gtkscrollbook.h>
#include <pidgin/gtksmiley-manager.h>
#include <pidgin/gtksmiley-theme.h>
#include <pidgin/gtkstatus-icon-theme.h>
#include <pidgin/gtkstatusbox.h>
#include <pidgin/pidginstock.h>
#include <pidgin/gtkutils.h>
#include <pidgin/gtkwhiteboard.h>
#include <pidgin/gtkxfer.h>
#include <pidgin/minidialog.h>
#include <pidgin/pidginabout.h>
#include <pidgin/pidginaccountactionsmenu.h>
#include <pidgin/pidginaccountchooser.h>
#include <pidgin/pidginaccountfilterconnected.h>
#include <pidgin/pidginaccountfilterprotocol.h>
#include <pidgin/pidginaccountsmenu.h>
#include <pidgin/pidginaccountstore.h>
#include <pidgin/pidginactiongroup.h>
#include <pidgin/pidginapplication.h>
#include <pidgin/pidginattachment.h>
#include <pidgin/pidgincellrendererexpander.h>
#include <pidgin/pidginclosebutton.h>
#include <pidgin/pidgincontactcompletion.h>
#include <pidgin/pidginconversationwindow.h>
#include <pidgin/pidgincontactlist.h>
#include <pidgin/pidgincore.h>
#include <pidgin/pidgincredentialproviderstore.h>
#include <pidgin/pidgincredentialspage.h>
#include <pidgin/pidgindialog.h>
#include <pidgin/pidgindebug.h>
#include <pidgin/pidgingdkpixbuf.h>
#include <pidgin/pidginicon.h>
#include <pidgin/pidgininvitedialog.h>
#include <pidgin/pidginlog.h>
#include <pidgin/pidginmenutray.h>
#include <pidgin/pidginmessage.h>
#include <pidgin/pidginmooddialog.h>
#include <pidgin/pidginplugininfo.h>
#include <pidgin/pidginpluginsdialog.h>
#include <pidgin/pidginpluginsmenu.h>
#include <pidgin/pidginpresence.h>
#include <pidgin/pidginpresenceicon.h>
#include <pidgin/pidginprotocolchooser.h>
#include <pidgin/pidginprotocolstore.h>
#include <pidgin/pidginstylecontext.h>
#include <pidgin/pidgintalkatu.h>
#include <pidgin/pidgintooltip.h>
#include <pidgin/pidginwindow.h>
#include <pidgin/pidginenums.h>

#undef PIDGIN_GLOBAL_HEADER_INSIDE

#endif /* PIDGIN_PIDGIN_H */
