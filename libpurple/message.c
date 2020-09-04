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
 */

#include <glib/gi18n-lib.h>

#include "internal.h"

#include "debug.h"
#include "enums.h"
#include "message.h"
#include "purpleprivate.h"

/**
 * PurpleMessage:
 *
 * A message data container.
 */
struct _PurpleMessage
{
	GObject parent;
};

typedef struct {
	guint id;
	gchar *author;
	gchar *author_alias;
	gchar *recipient;
	gchar *contents;
	guint64 msgtime;
	PurpleMessageFlags flags;

	GHashTable *attachments;
} PurpleMessagePrivate;

enum
{
	PROP_0,
	PROP_ID,
	PROP_AUTHOR,
	PROP_AUTHOR_ALIAS,
	PROP_RECIPIENT,
	PROP_CONTENTS,
	PROP_TIME,
	PROP_FLAGS,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST];

static GHashTable *messages = NULL;

G_DEFINE_TYPE_WITH_PRIVATE(PurpleMessage, purple_message, G_TYPE_OBJECT)

/******************************************************************************
 * API implementation
 ******************************************************************************/

PurpleMessage *
purple_message_new_outgoing(const gchar *who, const gchar *contents,
	PurpleMessageFlags flags)
{
	g_warn_if_fail(!(flags & PURPLE_MESSAGE_RECV));
	g_warn_if_fail(!(flags & PURPLE_MESSAGE_SYSTEM));

	flags |= PURPLE_MESSAGE_SEND;

	/* who may be NULL for outgoing MUC messages */
	return g_object_new(PURPLE_TYPE_MESSAGE,
		"author-alias", _("Me"),
		"recipient", who,
		"contents", contents,
		"time", (guint64)time(NULL),
		"flags", flags,
		NULL);
}

PurpleMessage *
purple_message_new_incoming(const gchar *who, const gchar *contents,
	PurpleMessageFlags flags, guint64 timestamp)
{
	g_warn_if_fail(!(flags & PURPLE_MESSAGE_SEND));
	g_warn_if_fail(!(flags & PURPLE_MESSAGE_SYSTEM));

	flags |= PURPLE_MESSAGE_RECV;

	if (timestamp == 0)
		timestamp = time(NULL);

	return g_object_new(PURPLE_TYPE_MESSAGE,
		"author", who,
		"author-alias", who,
		"contents", contents,
		"time", timestamp,
		"flags", flags,
		NULL);
}

PurpleMessage *
purple_message_new_system(const gchar *contents, PurpleMessageFlags flags)
{
	g_warn_if_fail(!(flags & PURPLE_MESSAGE_SEND));
	g_warn_if_fail(!(flags & PURPLE_MESSAGE_RECV));

	flags |= PURPLE_MESSAGE_SYSTEM;

	return g_object_new(PURPLE_TYPE_MESSAGE,
		"contents", contents,
		"time", (guint64)time(NULL),
		"flags", flags,
		NULL);
}

guint
purple_message_get_id(PurpleMessage *msg)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(msg), 0);

	priv = purple_message_get_instance_private(msg);
	return priv->id;
}

PurpleMessage *
purple_message_find_by_id(guint id)
{
	g_return_val_if_fail(id > 0, NULL);

	return g_hash_table_lookup(messages, GINT_TO_POINTER(id));
}

const gchar *
purple_message_get_author(PurpleMessage *msg)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(msg), NULL);

	priv = purple_message_get_instance_private(msg);
	return priv->author;
}

const gchar *
purple_message_get_recipient(PurpleMessage *msg)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(msg), NULL);

	priv = purple_message_get_instance_private(msg);
	return priv->recipient;
}

void
purple_message_set_author_alias(PurpleMessage *msg, const gchar *alias)
{
	g_object_set(msg, "author-alias", alias, NULL);
}

const gchar *
purple_message_get_author_alias(PurpleMessage *msg)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(msg), NULL);
	priv = purple_message_get_instance_private(msg);

	if (priv->author_alias == NULL)
		return purple_message_get_author(msg);

	return priv->author_alias;
}

void
purple_message_set_contents(PurpleMessage *msg, const gchar *cont)
{
	g_object_set(msg, "contents", cont, NULL);
}

const gchar *
purple_message_get_contents(PurpleMessage *msg)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(msg), NULL);

	priv = purple_message_get_instance_private(msg);
	return priv->contents;
}

gboolean
purple_message_is_empty(PurpleMessage *msg)
{
	const gchar *cont = purple_message_get_contents(msg);

	return (cont == NULL || cont[0] == '\0');
}

void
purple_message_set_time(PurpleMessage *msg, guint64 msgtime)
{
	g_object_set(msg, "time", msgtime, NULL);
}

guint64
purple_message_get_time(PurpleMessage *msg)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(msg), 0);

	priv = purple_message_get_instance_private(msg);
	return priv->msgtime;
}

void
purple_message_set_flags(PurpleMessage *msg, PurpleMessageFlags flags)
{
	g_object_set(msg, "flags", flags, NULL);
}

PurpleMessageFlags
purple_message_get_flags(PurpleMessage *msg)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(msg), 0);

	priv = purple_message_get_instance_private(msg);
	return priv->flags;
}

gboolean
purple_message_add_attachment(PurpleMessage *message,
                              PurpleAttachment *attachment)
{
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(message), FALSE);
	g_return_val_if_fail(PURPLE_IS_ATTACHMENT(attachment), FALSE);

	priv = purple_message_get_instance_private(message);

	return g_hash_table_insert(priv->attachments,
	                           purple_attachment_get_hash_key(attachment),
	                           g_object_ref(G_OBJECT(attachment)));
}

gboolean
purple_message_remove_attachment(PurpleMessage *message, guint64 id) {
	PurpleMessagePrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(message), FALSE);

	priv = purple_message_get_instance_private(message);

	return g_hash_table_remove(priv->attachments, &id);
}

PurpleAttachment *
purple_message_get_attachment(PurpleMessage *message, guint64 id) {
	PurpleMessagePrivate *priv = NULL;
	PurpleAttachment *attachment = NULL;

	g_return_val_if_fail(PURPLE_IS_MESSAGE(message), NULL);

	priv = purple_message_get_instance_private(message);

	attachment = g_hash_table_lookup(priv->attachments, &id);

	if(PURPLE_IS_ATTACHMENT(attachment)) {
		return PURPLE_ATTACHMENT(g_object_ref(G_OBJECT(attachment)));
	}

	return NULL;
}

void
purple_message_foreach_attachment(PurpleMessage *message,
                                  PurpleAttachmentForeachFunc func,
                                  gpointer data)
{
	PurpleMessagePrivate *priv = NULL;
	GHashTableIter iter;
	gpointer value;

	g_return_if_fail(PURPLE_IS_MESSAGE(message));
	g_return_if_fail(func != NULL);

	g_hash_table_iter_init(&iter, priv->attachments);
	while(g_hash_table_iter_next(&iter, NULL, &value)) {
		func(PURPLE_ATTACHMENT(value), data);
	}
}

void
purple_message_clear_attachments(PurpleMessage *message) {
	PurpleMessagePrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_MESSAGE(message));

	priv = purple_message_get_instance_private(message);

	g_hash_table_remove_all(priv->attachments);
}

/******************************************************************************
 * Object stuff
 ******************************************************************************/

static void
purple_message_init(PurpleMessage *msg)
{
	static guint max_id = 0;

	PurpleMessagePrivate *priv = purple_message_get_instance_private(msg);

	priv->attachments = g_hash_table_new_full(g_int64_hash, g_int64_equal,
	                                          NULL, g_object_unref);

	priv->id = ++max_id;
	g_hash_table_insert(messages, GINT_TO_POINTER(max_id), msg);
}

static void
purple_message_finalize(GObject *obj)
{
	PurpleMessage *message = PURPLE_MESSAGE(obj);
	PurpleMessagePrivate *priv = purple_message_get_instance_private(message);

	g_free(priv->author);
	g_free(priv->author_alias);
	g_free(priv->recipient);
	g_free(priv->contents);

	g_hash_table_destroy(priv->attachments);

	G_OBJECT_CLASS(purple_message_parent_class)->finalize(obj);
}

static void
purple_message_get_property(GObject *object, guint par_id, GValue *value,
	GParamSpec *pspec)
{
	PurpleMessage *message = PURPLE_MESSAGE(object);
	PurpleMessagePrivate *priv = purple_message_get_instance_private(message);

	switch (par_id) {
		case PROP_ID:
			g_value_set_uint(value, priv->id);
			break;
		case PROP_AUTHOR:
			g_value_set_string(value, priv->author);
			break;
		case PROP_AUTHOR_ALIAS:
			g_value_set_string(value, priv->author_alias);
			break;
		case PROP_RECIPIENT:
			g_value_set_string(value, priv->recipient);
			break;
		case PROP_CONTENTS:
			g_value_set_string(value, priv->contents);
			break;
		case PROP_TIME:
			g_value_set_uint64(value, priv->msgtime);
			break;
		case PROP_FLAGS:
			g_value_set_flags(value, priv->flags);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, par_id, pspec);
			break;
	}
}

static void
purple_message_set_property(GObject *object, guint par_id, const GValue *value,
	GParamSpec *pspec)
{
	PurpleMessage *message = PURPLE_MESSAGE(object);
	PurpleMessagePrivate *priv = purple_message_get_instance_private(message);

	switch (par_id) {
		case PROP_AUTHOR:
			g_free(priv->author);
			priv->author = g_value_dup_string(value);
			break;
		case PROP_AUTHOR_ALIAS:
			g_free(priv->author_alias);
			priv->author_alias = g_value_dup_string(value);
			break;
		case PROP_RECIPIENT:
			g_free(priv->recipient);
			priv->recipient = g_value_dup_string(value);
			break;
		case PROP_CONTENTS:
			g_free(priv->contents);
			priv->contents = g_value_dup_string(value);
			break;
		case PROP_TIME:
			priv->msgtime = g_value_get_uint64(value);
			break;
		case PROP_FLAGS:
			priv->flags = g_value_get_flags(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, par_id, pspec);
			break;
	}
}

static void
purple_message_class_init(PurpleMessageClass *klass)
{
	GObjectClass *gobj_class = G_OBJECT_CLASS(klass);

	gobj_class->finalize = purple_message_finalize;
	gobj_class->get_property = purple_message_get_property;
	gobj_class->set_property = purple_message_set_property;

	properties[PROP_ID] = g_param_spec_uint("id",
		"ID", "The session-unique message id",
		0, G_MAXUINT, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	properties[PROP_AUTHOR] = g_param_spec_string("author",
		"Author", "The username of the person, who sent the message.",
		NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	properties[PROP_AUTHOR_ALIAS] = g_param_spec_string("author-alias",
		"Author's alias", "The alias of the person, who sent the "
		"message. For outgoing messages, it's your alias.",
		NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	properties[PROP_RECIPIENT] = g_param_spec_string("recipient",
		"Recipient", "The username of the recipient.",
		NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	properties[PROP_CONTENTS] = g_param_spec_string("contents",
		"Contents", "The message text",
		NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	properties[PROP_TIME] = g_param_spec_uint64("time",
		"Time", "Message timestamp",
		0, G_MAXUINT64, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	properties[PROP_FLAGS] = g_param_spec_flags("flags",
		"Flags", "Bitwise set of #PurpleMessageFlags flags",
		PURPLE_TYPE_MESSAGE_FLAGS, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobj_class, PROP_LAST, properties);
}

void
_purple_message_init(void)
{
	messages = g_hash_table_new_full(g_direct_hash, g_direct_equal,
		NULL, g_object_unref);
}

void
_purple_message_uninit(void)
{
	g_hash_table_destroy(messages);
	messages = NULL;
}
