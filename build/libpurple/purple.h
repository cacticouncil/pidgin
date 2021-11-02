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

#ifndef PURPLE_PURPLE_H
#define PURPLE_PURPLE_H

#ifndef __GI_SCANNER__ /* hide this bit from g-ir-scanner */
# ifdef PURPLE_COMPILATION
#  error "libpurple source files should not be including purple.h"
# endif /* PURPLE_COMPILATION */
#endif /* __GI_SCANNER__ */

#include <glib.h>

#ifndef PURPLE_GLOBAL_HEADER_INSIDE
# define PURPLE_GLOBAL_HEADER_INSIDE
#endif /* PURPLE_GLOBAL_HEADER_INSIDE */

#include <libpurple/account.h>
#include <libpurple/accounts.h>
#include <libpurple/action.h>
#include <libpurple/blistnode.h>
#include <libpurple/buddy.h>
#include <libpurple/buddylist.h>
#include <libpurple/buddyicon.h>
#include <libpurple/chat.h>
#include <libpurple/circularbuffer.h>
#include <libpurple/cmds.h>
#include <libpurple/connection.h>
#include <libpurple/contact.h>
#include <libpurple/conversation.h>
#include <libpurple/conversationtypes.h>
#include <libpurple/conversations.h>
#include <libpurple/core.h>
#include <libpurple/countingnode.h>
#include <libpurple/debug.h>
#include <libpurple/eventloop.h>
#include <libpurple/group.h>
#include <libpurple/idle.h>
#include <libpurple/image.h>
#include <libpurple/image-store.h>
#include <libpurple/keyring.h>
#include <libpurple/log.h>
#include <libpurple/media.h>
#include <libpurple/mediamanager.h>
#include <libpurple/memorypool.h>
#include <libpurple/nat-pmp.h>
#include <libpurple/network.h>
#include <libpurple/notify.h>
#include <libpurple/plugins.h>
#include <libpurple/pluginpref.h>
#include <libpurple/prefs.h>
#include <libpurple/proxy.h>
#include <libpurple/protocol.h>
#include <libpurple/protocols.h>
#include <libpurple/purple-gio.h>
#include <libpurple/purpleaccountoption.h>
#include <libpurple/purpleaccountpresence.h>
#include <libpurple/purpleaccountusersplit.h>
#include <libpurple/purpleattentiontype.h>
#include <libpurple/purplebuddypresence.h>
#include <libpurple/purplechatuser.h>
#include <libpurple/purplecredentialmanager.h>
#include <libpurple/purplecredentialprovider.h>
#include <libpurple/purpleimconversation.h>
#include <libpurple/purpleattachment.h>
#include <libpurple/purplekeyvaluepair.h>
#include <libpurple/purplemarkup.h>
#include <libpurple/purplemessage.h>
#include <libpurple/purplenoopcredentialprovider.h>
#include <libpurple/purpleoptions.h>
#include <libpurple/purplepresence.h>
#include <libpurple/purpleprotocolattention.h>
#include <libpurple/purpleprotocolchat.h>
#include <libpurple/purpleprotocolclient.h>
#include <libpurple/purpleprotocolfactory.h>
#include <libpurple/purpleprotocolim.h>
#include <libpurple/purpleprotocolmedia.h>
#include <libpurple/purpleprotocolmanager.h>
#include <libpurple/purpleprotocolprivacy.h>
#include <libpurple/purpleprotocolserver.h>
#include <libpurple/purpleuiinfo.h>
#include <libpurple/purplewhiteboard.h>
#include <libpurple/purplewhiteboardops.h>
#include <libpurple/purplewhiteboarduiops.h>
#include <libpurple/queuedoutputstream.h>
#include <libpurple/request.h>
#include <libpurple/request-datasheet.h>
#include <libpurple/roomlist.h>
#include <libpurple/savedstatuses.h>
#include <libpurple/server.h>
#include <libpurple/signals.h>
#include <libpurple/smiley-custom.h>
#include <libpurple/smiley-list.h>
#include <libpurple/smiley-parser.h>
#include <libpurple/smiley-theme.h>
#include <libpurple/smiley.h>
#include <libpurple/status.h>
#include <libpurple/stun.h>
#include <libpurple/tests.h>
#include <libpurple/theme.h>
#include <libpurple/theme-loader.h>
#include <libpurple/theme-manager.h>
#include <libpurple/trie.h>
#include <libpurple/upnp.h>
#include <libpurple/util.h>
#include <libpurple/xfer.h>
#include <libpurple/xmlnode.h>
#include <libpurple/media-gst.h>
#include <libpurple/media/backend-iface.h>
#include <libpurple/media/candidate.h>
#include <libpurple/media/codec.h>
#include <libpurple/media/enum-types.h>
#include <libpurple/version.h>
#include <libpurple/enums.h>

#undef PURPLE_GLOBAL_HEADER_INSIDE

#endif /* PURPLE_PURPLE_H */
