/*
 * purple - Jabber Protocol Plugin
 *
 * Copyright (C) 2007, Andreas Monitzer <andy@monitzer.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307	 USA
 *
 */

#ifndef _PURPLE_JABBER_PEP_H_
#define _PURPLE_JABBER_PEP_H_

#include "jabber.h"
#include "message.h"
#include "buddy.h"

void jabber_pep_init(void);

void jabber_pep_init_actions(GList **m);

/*
 * Callback for receiving PEP events.
 *
 * @parameter js	The JabberStream this item was received on
 * @parameter items	The &lt;items/>-tag with the &lt;item/>-children
 */
typedef void (JabberPEPHandler)(JabberStream *js, const char *from, xmlnode *items);

/*
 * Registers a callback for PEP events. Also automatically announces this receiving capability via disco#info.
 * Don't forget to use jabber_add_feature when supporting the sending of PEP events of this type.
 *
 * @parameter shortname		A short name for this feature for XEP-0115. It has no semantic meaning, it just has to be unique.
 * @parameter xmlns		The namespace for this event
 * @parameter handlerfunc	The callback to be used when receiving an event with this namespace
 */
void jabber_pep_register_handler(const char *shortname, const char *xmlns, JabberPEPHandler handlerfunc);

/*
 * Request a specific item from another PEP node.
 *
 * @parameter js	The JabberStream that should be used
 * @parameter to	The target PEP node
 * @parameter node	The node name of the item that is requested
 * @parameter id	The item id of the requested item (may be NULL)
 * @parameter cb	The callback to be used when this item is received
 *
 * The items element passed to the callback will be NULL if any error occured (like a permission error, node doesn't exist etc.)
 */
void jabber_pep_request_item(JabberStream *js, const char *to, const char *node, const char *id, JabberPEPHandler cb);

/*
 * Default callback that can be used for namespaces which should only be enabled when PEP is supported
 *
 * @parameter js	The JabberStream struct for this connection
 * @parameter shortname	The namespace's shortname (for caps), ignored.
 * @parameter namespace The namespace that's queried, ignored.
 *
 * @returns TRUE when PEP is enabled, FALSE otherwise
 */
gboolean jabber_pep_namespace_only_when_pep_enabled_cb(JabberStream *js, const gchar *shortname, const gchar *namespace);

void jabber_handle_event(JabberMessage *jm);

/*
 * Publishes PEP item(s)
 *
 * @parameter js	The JabberStream associated with the connection this event should be published
 * @parameter publish	The publish node. This could be for example &lt;publish node='http://jabber.org/protocol/tune'/> with an &lt;item/> as subnode
 */
void jabber_pep_publish(JabberStream *js, xmlnode *publish);

#endif /* _PURPLE_JABBER_PEP_H_ */
