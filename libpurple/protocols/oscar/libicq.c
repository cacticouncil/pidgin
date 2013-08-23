/* purple
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
 *
 */

/* libicq is the ICQ protocol plugin. It is linked against liboscar,
 * which contains all the shared implementation code with libaim
 */

#include "plugins.h"

#include "oscarcommon.h"

static GHashTable *
icq_get_account_text_table(PurpleAccount *account)
{
	GHashTable *table;
	table = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(table, "login_label", (gpointer)_("ICQ UIN..."));
	return table;
}

static PurpleProtocol protocol =
{
	"prpl-icq",				/* id */
	"ICQ",					/* name */
	sizeof(PurpleProtocol),       /* struct_size */
	OPT_PROTO_MAIL_CHECK | OPT_PROTO_IM_IMAGE | OPT_PROTO_INVITE_MESSAGE | OPT_PROTO_AUTHORIZATION_DENIED_MESSAGE,
	NULL,					/* user_splits */
	NULL,					/* protocol_options */
	{"gif,jpeg,bmp,ico", 0, 0, 64, 64, 7168, PURPLE_ICON_SCALE_SEND | PURPLE_ICON_SCALE_DISPLAY}, /* icon_spec */
	oscar_get_actions,		/* get_actions */
	oscar_list_icon_icq,	/* list_icon */
	oscar_list_emblem,		/* list_emblems */
	oscar_status_text,		/* status_text */
	oscar_tooltip_text,		/* tooltip_text */
	oscar_status_types,		/* status_types */
	oscar_blist_node_menu,	/* blist_node_menu */
	oscar_chat_info,		/* chat_info */
	oscar_chat_info_defaults, /* chat_info_defaults */
	oscar_login,			/* login */
	oscar_close,			/* close */
	oscar_send_im,			/* send_im */
	oscar_set_info,			/* set_info */
	oscar_send_typing,		/* send_typing */
	oscar_get_info,			/* get_info */
	oscar_set_status,		/* set_status */
	oscar_set_idle,			/* set_idle */
	oscar_change_passwd,	/* change_passwd */
	oscar_add_buddy,		/* add_buddy */
	NULL,					/* add_buddies */
	oscar_remove_buddy,		/* remove_buddy */
	NULL,					/* remove_buddies */
	NULL,		/* add_permit */
	oscar_add_deny,			/* add_deny */
	NULL,		/* rem_permit */
	oscar_rem_deny,			/* rem_deny */
	NULL,	/* set_permit_deny */
	oscar_join_chat,		/* join_chat */
	NULL,					/* reject_chat */
	oscar_get_chat_name,	/* get_chat_name */
	oscar_chat_invite,		/* chat_invite */
	oscar_chat_leave,		/* chat_leave */
	NULL,					/* chat_whisper */
	oscar_send_chat,		/* chat_send */
	oscar_keepalive,		/* keepalive */
	NULL,					/* register_user */
	NULL,					/* get_cb_info */
	oscar_alias_buddy,		/* alias_buddy */
	oscar_move_buddy,		/* group_buddy */
	oscar_rename_group,		/* rename_group */
	NULL,					/* buddy_free */
	oscar_convo_closed,		/* convo_closed */
	oscar_normalize,		/* normalize */
	oscar_set_icon,			/* set_buddy_icon */
	oscar_remove_group,		/* remove_group */
	NULL,					/* get_cb_real_name */
	NULL,					/* set_chat_topic */
	NULL,					/* find_blist_chat */
	NULL,					/* roomlist_get_list */
	NULL,					/* roomlist_cancel */
	NULL,					/* roomlist_expand_category */
	oscar_can_receive_file,	/* can_receive_file */
	oscar_send_file,		/* send_file */
	oscar_new_xfer,			/* new_xfer */
	oscar_offline_message,	/* offline_message */
	NULL,					/* whiteboard_protocol_ops */
	NULL,					/* send_raw */
	NULL,					/* roomlist_room_serialize */
	NULL,					/* unregister_user */
	NULL,					/* send_attention */
	NULL,					/* get_attention_types */
	icq_get_account_text_table, /* get_account_text_table */
	NULL,					/* initiate_media */
	NULL,					/* can_do_media */
	oscar_get_purple_moods, /* get_moods */
	NULL,					/* set_public_alias */
	NULL					/* get_public_alias */
};

static PurplePluginInfo *
plugin_query(GError **error)
{
	return purple_plugin_info_new(
		"id",           "prpl-icq",
		"name",         "ICQ",
		"version",      DISPLAY_VERSION,
		"category",     N_("Protocol"),
		"summary",      N_("ICQ Protocol Plugin"),
		"description",  N_("ICQ Protocol Plugin"),
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"flags",        GPLUGIN_PLUGIN_INFO_FLAGS_INTERNAL |
		                GPLUGIN_PLUGIN_INFO_FLAGS_LOAD_ON_QUERY,
		NULL
	);
}

static gboolean
plugin_load(PurplePlugin *plugin, GError **error)
{
	PurpleAccountOption *option;

	oscar_init(&protocol, TRUE);

	option = purple_account_option_string_new(_("Encoding"), "encoding", OSCAR_DEFAULT_CUSTOM_ENCODING);
	protocol.protocol_options = g_list_append(protocol.protocol_options, option);

	purple_protocols_add(&protocol);

	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin, GError **error)
{
	purple_protocols_remove(&protocol);

	return TRUE;
}

PURPLE_PLUGIN_INIT(icq, plugin_query, plugin_load, plugin_unload);
