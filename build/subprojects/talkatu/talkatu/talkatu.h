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

#ifndef TALKATU_H
#define TALKATU_H

#ifndef __GI_SCANNER__
# ifdef TALKATU_COMPILATION
#  error "talkatu sources should not include talkatu.h"
# endif /* TALKATU_COMPILATION */
#endif /* __GI_SCANNER__ */

#ifndef TALKATU_GLOBAL_HEADER_INSIDE
# define TALKATU_GLOBAL_HEADER_INSIDE
#endif /* TALKATU_GLOBAL_HEADER_INSIDE */


#include <talkatu/talkatuactiongroup.h>
#include <talkatu/talkatuattachment.h>
#include <talkatu/talkatuattachmentdialog.h>
#include <talkatu/talkatuattachmentpreview.h>
#include <talkatu/talkatubuffer.h>
#include <talkatu/talkatucodeset.h>
#include <talkatu/talkatucore.h>
#include <talkatu/talkatueditor.h>
#include <talkatu/talkatuhistory.h>
#include <talkatu/talkatuhistoryrow.h>
#include <talkatu/talkatuhtmlbuffer.h>
#include <talkatu/talkatuhtmlpangorenderer.h>
#include <talkatu/talkatuhtmlrenderer.h>
#include <talkatu/talkatuinput.h>
#include <talkatu/talkatulinkdialog.h>
#include <talkatu/talkatumarkdownbuffer.h>
#include <talkatu/talkatumarkup.h>
#include <talkatu/talkatumenutoolbutton.h>
#include <talkatu/talkatumessage.h>
#include <talkatu/talkatumessageactions.h>
#include <talkatu/talkatuscrolledwindow.h>
#include <talkatu/talkatusimpleattachment.h>
#include <talkatu/talkatutag.h>
#include <talkatu/talkatutagtable.h>
#include <talkatu/talkatutoolbar.h>
#include <talkatu/talkatutooldrawer.h>
#include <talkatu/talkatutypinglabel.h>
#include <talkatu/talkatuview.h>
#include <talkatu/talkatuwholebuffer.h>
#include <talkatu/talkatuenums.h>
#include <talkatu/talkatuversion.h>

#undef TALKATU_GLOBAL_HEADER_INSIDE

#endif /* TALKATU_H */
