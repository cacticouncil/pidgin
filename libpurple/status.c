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

#include "purpleprivate.h"

#include "buddylist.h"
#include "core.h"
#include "debug.h"
#include "notify.h"
#include "prefs.h"
#include "purpleaccountpresence.h"
#include "purplebuddypresence.h"
#include "status.h"

/*
 * A type of status.
 */
struct _PurpleStatusType
{
	int box_count;

	PurpleStatusPrimitive primitive;

	char *id;
	char *name;

	gboolean saveable;
	gboolean user_settable;
	gboolean independent;

	GList *attrs;
};

/*
 * A status attribute.
 */
struct _PurpleStatusAttribute
{
	char *id;
	char *name;
	GValue *value_type;
};

struct _PurpleStatus
{
	GObject parent;

	PurpleStatusType *status_type;
	PurplePresence *presence;

	gboolean active;

	/*
	 * The current values of the attributes for this status.  The
	 * key is a string containing the name of the attribute.  It is
	 * a borrowed reference from the list of attrs in the
	 * PurpleStatusType.  The value is a GValue.
	 */
	GHashTable *attr_values;
};

/* GObject property enums */
enum
{
	PROP_0,
	PROP_STATUS_TYPE,
	PROP_PRESENCE,
	PROP_ACTIVE,
	PROP_LAST
};

typedef struct
{
	PurpleAccount *account;
	char *name;
} PurpleStatusBuddyKey;

static GParamSpec *properties[PROP_LAST];

G_DEFINE_TYPE(PurpleStatus, purple_status, G_TYPE_OBJECT);

static int primitive_scores[] =
{
	0,      /* unset                    */
	-500,   /* offline                  */
	100,    /* available                */
	-75,    /* unavailable              */
	-50,    /* invisible                */
	-100,   /* away                     */
	-200,   /* extended away            */
	-400,   /* mobile                   */
	0,      /* tune                     */
	0,      /* mood                     */
	-10,    /* idle, special case.      */
	-5,     /* idle time, special case. */
	10      /* Offline messageable      */
};

#define SCORE_IDLE      9
#define SCORE_IDLE_TIME 10
#define SCORE_OFFLINE_MESSAGE 11

/**************************************************************************
 * PurpleStatusPrimitive API
 **************************************************************************/
static struct PurpleStatusPrimitiveMap
{
	PurpleStatusPrimitive type;
	const char *id;
	const char *name;

} const status_primitive_map[] =
{
	{ PURPLE_STATUS_UNSET,           "unset",           N_("Unset")               },
	{ PURPLE_STATUS_OFFLINE,         "offline",         N_("Offline")             },
	{ PURPLE_STATUS_AVAILABLE,       "available",       N_("Available")           },
	{ PURPLE_STATUS_UNAVAILABLE,     "unavailable",     N_("Do not disturb")      },
	{ PURPLE_STATUS_INVISIBLE,       "invisible",       N_("Invisible")           },
	{ PURPLE_STATUS_AWAY,            "away",            N_("Away")                },
	{ PURPLE_STATUS_EXTENDED_AWAY,   "extended_away",   N_("Extended away")       },
	{ PURPLE_STATUS_MOBILE,          "mobile",          N_("Mobile")              },
	{ PURPLE_STATUS_TUNE,            "tune",            N_("Listening to music"), },
	{ PURPLE_STATUS_MOOD,            "mood",            N_("Feeling")             },
};

int *
_purple_statuses_get_primitive_scores(void)
{
	return primitive_scores;
}

const char *
purple_primitive_get_id_from_type(PurpleStatusPrimitive type)
{
    int i;

    for (i = 0; i < PURPLE_STATUS_NUM_PRIMITIVES; i++)
    {
		if (type == status_primitive_map[i].type)
			return status_primitive_map[i].id;
    }

    return status_primitive_map[0].id;
}

const char *
purple_primitive_get_name_from_type(PurpleStatusPrimitive type)
{
    int i;

    for (i = 0; i < PURPLE_STATUS_NUM_PRIMITIVES; i++)
    {
	if (type == status_primitive_map[i].type)
		return _(status_primitive_map[i].name);
    }

    return _(status_primitive_map[0].name);
}

PurpleStatusPrimitive
purple_primitive_get_type_from_id(const char *id)
{
    int i;

    g_return_val_if_fail(id != NULL, PURPLE_STATUS_UNSET);

    for (i = 0; i < PURPLE_STATUS_NUM_PRIMITIVES; i++)
    {
		if (purple_strequal(id, status_primitive_map[i].id))
            return status_primitive_map[i].type;
    }

    return status_primitive_map[0].type;
}


/**************************************************************************
 * PurpleStatusType API
 **************************************************************************/
PurpleStatusType *
purple_status_type_new_full(PurpleStatusPrimitive primitive, const char *id,
						  const char *name, gboolean saveable,
						  gboolean user_settable, gboolean independent)
{
	PurpleStatusType *status_type;

	g_return_val_if_fail(primitive != PURPLE_STATUS_UNSET, NULL);

	status_type = g_new0(PurpleStatusType, 1);

	status_type->primitive     = primitive;
	status_type->saveable      = saveable;
	status_type->user_settable = user_settable;
	status_type->independent   = independent;

	if (id != NULL)
		status_type->id = g_strdup(id);
	else
		status_type->id = g_strdup(purple_primitive_get_id_from_type(primitive));

	if (name != NULL)
		status_type->name = g_strdup(name);
	else
		status_type->name = g_strdup(purple_primitive_get_name_from_type(primitive));

	return status_type;
}

PurpleStatusType *
purple_status_type_new(PurpleStatusPrimitive primitive, const char *id,
					 const char *name, gboolean user_settable)
{
	g_return_val_if_fail(primitive != PURPLE_STATUS_UNSET, NULL);

	return purple_status_type_new_full(primitive, id, name, TRUE,
			user_settable, FALSE);
}

static void
status_type_add_attr(PurpleStatusType *status_type, const char *id,
		const char *name, GValue *value)
{
	PurpleStatusAttribute *attr;

	g_return_if_fail(status_type != NULL);
	g_return_if_fail(id          != NULL);
	g_return_if_fail(name        != NULL);
	g_return_if_fail(value       != NULL);

	attr = purple_status_attribute_new(id, name, value);

	status_type->attrs = g_list_append(status_type->attrs, attr);
}

static void
status_type_add_attrs_vargs(PurpleStatusType *status_type, va_list args)
{
	const char *id, *name;
	GValue *value;

	g_return_if_fail(status_type != NULL);

	while ((id = va_arg(args, const char *)) != NULL)
	{
		name = va_arg(args, const char *);
		g_return_if_fail(name != NULL);

		value = va_arg(args, GValue *);
		g_return_if_fail(value != NULL);

		status_type_add_attr(status_type, id, name, value);
	}
}

PurpleStatusType *
purple_status_type_new_with_attrs(PurpleStatusPrimitive primitive,
		const char *id, const char *name,
		gboolean saveable, gboolean user_settable,
		gboolean independent, const char *attr_id,
		const char *attr_name, GValue *attr_value,
		...)
{
	PurpleStatusType *status_type;
	va_list args;

	g_return_val_if_fail(primitive  != PURPLE_STATUS_UNSET, NULL);
	g_return_val_if_fail(attr_id    != NULL,              NULL);
	g_return_val_if_fail(attr_name  != NULL,              NULL);
	g_return_val_if_fail(attr_value != NULL,              NULL);

	status_type = purple_status_type_new_full(primitive, id, name, saveable,
			user_settable, independent);

	/* Add the first attribute */
	status_type_add_attr(status_type, attr_id, attr_name, attr_value);

	va_start(args, attr_value);
	status_type_add_attrs_vargs(status_type, args);
	va_end(args);

	return status_type;
}

PurpleStatusType *
purple_status_type_new_with_attrsv(PurpleStatusPrimitive primitive,
                                   const char *id, const char *name,
                                   gboolean saveable, gboolean user_settable,
                                   gboolean independent, guint n_attributes,
                                   const char *attr_id[],
                                   const char *attr_name[],
                                   GValue *attr_value[])
{
	PurpleStatusType *status_type;

	g_return_val_if_fail(primitive != PURPLE_STATUS_UNSET, NULL);
	g_return_val_if_fail(attr_id != NULL, NULL);
	g_return_val_if_fail(attr_name != NULL, NULL);
	g_return_val_if_fail(attr_value != NULL, NULL);

	status_type = purple_status_type_new_full(primitive, id, name, saveable,
	                                          user_settable, independent);

	for(guint i = 0; i < n_attributes; i++) {
		if(attr_id[i] == NULL || attr_name[i] == NULL ||
		   attr_value[i] == NULL)
		{
			purple_status_type_destroy(status_type);
			g_return_val_if_fail(attr_id[i] != NULL, NULL);
			g_return_val_if_fail(attr_name[i] != NULL, NULL);
			g_return_val_if_fail(attr_value[i] != NULL, NULL);
			return NULL;
		}

		status_type_add_attr(status_type, attr_id[i], attr_name[i],
		                     attr_value[i]);
	}

	return status_type;
}

void
purple_status_type_destroy(PurpleStatusType *status_type)
{
	g_return_if_fail(status_type != NULL);

	g_free(status_type->id);
	g_free(status_type->name);

	g_list_free_full(status_type->attrs,
	                 (GDestroyNotify)purple_status_attribute_destroy);

	g_free(status_type);
}

PurpleStatusPrimitive
purple_status_type_get_primitive(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, PURPLE_STATUS_UNSET);

	return status_type->primitive;
}

const char *
purple_status_type_get_id(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, NULL);

	return status_type->id;
}

const char *
purple_status_type_get_name(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, NULL);

	return status_type->name;
}

gboolean
purple_status_type_is_saveable(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, FALSE);

	return status_type->saveable;
}

gboolean
purple_status_type_is_user_settable(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, FALSE);

	return status_type->user_settable;
}

gboolean
purple_status_type_is_independent(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, FALSE);

	return status_type->independent;
}

gboolean
purple_status_type_is_exclusive(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, FALSE);

	return !status_type->independent;
}

gboolean
purple_status_type_is_available(const PurpleStatusType *status_type)
{
	PurpleStatusPrimitive primitive;

	g_return_val_if_fail(status_type != NULL, FALSE);

	primitive = purple_status_type_get_primitive(status_type);

	return (primitive == PURPLE_STATUS_AVAILABLE);
}

PurpleStatusAttribute *
purple_status_type_get_attr(const PurpleStatusType *status_type, const char *id)
{
	GList *l;

	g_return_val_if_fail(status_type != NULL, NULL);
	g_return_val_if_fail(id          != NULL, NULL);

	for (l = status_type->attrs; l != NULL; l = l->next)
	{
		PurpleStatusAttribute *attr = (PurpleStatusAttribute *)l->data;

		if (purple_strequal(purple_status_attribute_get_id(attr), id))
			return attr;
	}

	return NULL;
}

GList *
purple_status_type_get_attrs(const PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, NULL);

	return status_type->attrs;
}

const PurpleStatusType *
purple_status_type_find_with_id(GList *status_types, const char *id)
{
	PurpleStatusType *status_type;

	g_return_val_if_fail(id != NULL, NULL);

	while (status_types != NULL)
	{
		status_type = status_types->data;

		if (purple_strequal(id, status_type->id))
			return status_type;

		status_types = status_types->next;
	}

	return NULL;
}


/**************************************************************************
* PurpleStatusAttribute API
**************************************************************************/
PurpleStatusAttribute *
purple_status_attribute_new(const char *id, const char *name, GValue *value_type)
{
	PurpleStatusAttribute *attr;

	g_return_val_if_fail(id         != NULL, NULL);
	g_return_val_if_fail(name       != NULL, NULL);
	g_return_val_if_fail(value_type != NULL, NULL);

	attr = g_new0(PurpleStatusAttribute, 1);

	attr->id         = g_strdup(id);
	attr->name       = g_strdup(name);
	attr->value_type = value_type;

	return attr;
}

void
purple_status_attribute_destroy(PurpleStatusAttribute *attr)
{
	g_return_if_fail(attr != NULL);

	g_free(attr->id);
	g_free(attr->name);

	purple_value_free(attr->value_type);

	g_free(attr);
}

const char *
purple_status_attribute_get_id(const PurpleStatusAttribute *attr)
{
	g_return_val_if_fail(attr != NULL, NULL);

	return attr->id;
}

const char *
purple_status_attribute_get_name(const PurpleStatusAttribute *attr)
{
	g_return_val_if_fail(attr != NULL, NULL);

	return attr->name;
}

GValue *
purple_status_attribute_get_value(const PurpleStatusAttribute *attr)
{
	g_return_val_if_fail(attr != NULL, NULL);

	return attr->value_type;
}


/**************************************************************************
* PurpleStatus API
**************************************************************************/
static void
notify_status_update(PurplePresence *presence, PurpleStatus *old_status,
					 PurpleStatus *new_status)
{
	if (PURPLE_IS_ACCOUNT_PRESENCE(presence))
	{
		PurpleAccount *account = purple_account_presence_get_account(
				PURPLE_ACCOUNT_PRESENCE(presence));
		PurpleAccountUiOps *ops = purple_accounts_get_ui_ops();

		if (purple_account_get_enabled(account))
			purple_protocol_change_account_status(account, old_status, new_status);

		if (ops != NULL && ops->status_changed != NULL)
		{
			ops->status_changed(account, new_status);
		}
	}
}

static void
status_has_changed(PurpleStatus *status)
{
	PurplePresence *presence;
	PurpleStatus *old_status;

	presence   = purple_status_get_presence(status);

	/*
	 * If this status is exclusive, then we must be setting it to "active."
	 * Since we are setting it to active, we want to set the currently
	 * active status to "inactive."
	 */
	if (purple_status_is_exclusive(status))
	{
		old_status = purple_presence_get_active_status(presence);
		if (old_status != NULL && (old_status != status)) {
			old_status->active = FALSE;
			g_object_notify_by_pspec(G_OBJECT(old_status),
					properties[PROP_ACTIVE]);
		}
	}
	else
		old_status = NULL;

	g_object_set(presence, "active-status", status, NULL);
	g_object_notify_by_pspec(G_OBJECT(status), properties[PROP_ACTIVE]);

	notify_status_update(presence, old_status, status);
}

static void
status_set_attr_boolean(PurpleStatus *status, const char *id,
		gboolean value)
{
	GValue *attr_value;

	g_return_if_fail(PURPLE_IS_STATUS(status));
	g_return_if_fail(id != NULL);

	/* Make sure this attribute exists and is the correct type. */
	attr_value = purple_status_get_attr_value(status, id);
	g_return_if_fail(attr_value != NULL);
	g_return_if_fail(G_VALUE_TYPE(attr_value) == G_TYPE_BOOLEAN);

	g_value_set_boolean(attr_value, value);
}

static void
status_set_attr_int(PurpleStatus *status, const char *id, int value)
{
	GValue *attr_value;

	g_return_if_fail(PURPLE_IS_STATUS(status));
	g_return_if_fail(id != NULL);

	/* Make sure this attribute exists and is the correct type. */
	attr_value = purple_status_get_attr_value(status, id);
	g_return_if_fail(attr_value != NULL);
	g_return_if_fail(G_VALUE_TYPE(attr_value) == G_TYPE_INT);

	g_value_set_int(attr_value, value);
}

static void
status_set_attr_string(PurpleStatus *status, const char *id,
		const char *value)
{
	GValue *attr_value;

	g_return_if_fail(PURPLE_IS_STATUS(status));
	g_return_if_fail(id != NULL);

	/* Make sure this attribute exists and is the correct type. */
	attr_value = purple_status_get_attr_value(status, id);
	/* This used to be g_return_if_fail, but it's failing a LOT, so
	 * let's generate a log error for now. */
	/* g_return_if_fail(attr_value != NULL); */
	if (attr_value == NULL) {
		purple_debug_error("status",
				 "Attempted to set status attribute '%s' for "
				 "status '%s', which is not legal.  Fix "
                                 "this!\n", id,
				 purple_status_type_get_name(purple_status_get_status_type(status)));
		return;
	}
	g_return_if_fail(G_VALUE_TYPE(attr_value) == G_TYPE_STRING);

	/* XXX: Check if the value has actually changed. If it has, and the status
	 * is active, should this trigger 'status_has_changed'? */
	g_value_set_string(attr_value, value);
}

void
purple_status_set_active(PurpleStatus *status, gboolean active)
{
	purple_status_set_active_with_attrs_dict(status, active, NULL);
}

/*
 * This used to parse the va_list directly, but now it creates a GList
 * and passes it to purple_status_set_active_with_attrs_list().  That
 * function was created because account.c needs to pass a GList of
 * attributes to the status API.
 */
void
purple_status_set_active_with_attrs(PurpleStatus *status, gboolean active, va_list args)
{
	GHashTable *attrs = purple_attrs_from_vargs(args);

	purple_status_set_active_with_attrs_dict(status, active, attrs);
	g_hash_table_destroy(attrs);
}

void
purple_status_set_active_with_attrs_dict(PurpleStatus *status, gboolean active,
									     GHashTable *attrs)
{
	gboolean changed = FALSE;
	GList *l;
	GList *specified_attr_ids = NULL;
	PurpleStatusType *status_type;

	g_return_if_fail(PURPLE_IS_STATUS(status));

	if(!active && purple_status_is_exclusive(status)) {
		purple_debug_error("status",
		                   "Cannot deactivate an exclusive status (%s).",
		                   purple_status_get_id(status));
		return;
	}

	if(status->active != active) {
		changed = TRUE;
	}
	status->active = active;

	if(attrs != NULL) {
		GHashTableIter iter;
		gpointer key, data;

		/* Set any attributes */
		g_hash_table_iter_init(&iter, attrs);
		while(g_hash_table_iter_next(&iter, &key, &data)) {
			gchar *id = (gchar *)key;
			GValue *value = purple_status_get_attr_value(status, id);
			if(value == NULL) {
				purple_debug_warning("status",
				                     "The attribute \"%s\" on the status "
				                     "\"%s\" is not supported.",
				                     id, status->status_type->name);

				/* Skip over the data and move on to the next attribute */
				continue;
			}

			specified_attr_ids = g_list_prepend(specified_attr_ids, id);

			if(G_VALUE_HOLDS_STRING(value)) {
				const gchar *string_data = (const gchar *)data;

				if(purple_strequal(string_data, g_value_get_string(value))) {
					continue;
				}

				status_set_attr_string(status, id, string_data);
				changed = TRUE;
			} else if(G_VALUE_HOLDS_INT(value)) {
				gint int_data = GPOINTER_TO_INT(data);

				if(int_data == g_value_get_int(value)) {
					continue;
				}

				status_set_attr_int(status, id, int_data);
				changed = TRUE;
			} else if(G_VALUE_HOLDS_BOOLEAN(value)) {
				gboolean boolean_data = GPOINTER_TO_INT(data);

				if(boolean_data == g_value_get_boolean(value)) {
					continue;
				}

				status_set_attr_boolean(status, id, boolean_data);
				changed = TRUE;
			}
		}
	}

	/* Reset any unspecified attributes to their default value */
	status_type = purple_status_get_status_type(status);
	l = purple_status_type_get_attrs(status_type);
	while(l != NULL) {
		PurpleStatusAttribute *attr;
		GList *found = NULL;
		GValue *default_value = NULL;
		default_value = NULL;

		attr = l->data;
		l = l->next;

		found = g_list_find_custom(specified_attr_ids, attr->id,
		                           (GCompareFunc)g_strcmp0);
		if(found != NULL) {
			continue;
		}

		default_value = purple_status_attribute_get_value(attr);
		if(G_VALUE_HOLDS_STRING(default_value)) {
			const gchar *cur = purple_status_get_attr_string(status,
			                                                 attr->id);
			const gchar *def = g_value_get_string(default_value);

			if (purple_strequal(cur, def)) {
				continue;
			}

			status_set_attr_string(status, attr->id, def);
		} else if(G_VALUE_HOLDS_INT(default_value)) {
			gint cur = purple_status_get_attr_int(status, attr->id);
			gint def = g_value_get_int(default_value);

			if(cur == def) {
				continue;
			}

			status_set_attr_int(status, attr->id, def);
		} else if(G_VALUE_HOLDS_BOOLEAN(default_value)) {
			gboolean cur = purple_status_get_attr_boolean(status, attr->id);
			gboolean def = g_value_get_boolean(default_value);

			if(cur == def) {
				continue;
			}

			status_set_attr_boolean(status, attr->id, def);
		}
		changed = TRUE;
	}
	g_list_free(specified_attr_ids);

	if(changed) {
		status_has_changed(status);
	}
}

PurpleStatusType *
purple_status_get_status_type(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), NULL);

	return status->status_type;
}

PurplePresence *
purple_status_get_presence(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), NULL);

	return status->presence;
}

const char *
purple_status_get_id(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), NULL);

	return purple_status_type_get_id(purple_status_get_status_type(status));
}

const char *
purple_status_get_name(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), NULL);

	return purple_status_type_get_name(purple_status_get_status_type(status));
}

gboolean
purple_status_is_independent(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), FALSE);

	return purple_status_type_is_independent(purple_status_get_status_type(status));
}

gboolean
purple_status_is_exclusive(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), FALSE);

	return purple_status_type_is_exclusive(purple_status_get_status_type(status));
}

gboolean
purple_status_is_available(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), FALSE);

	return purple_status_type_is_available(purple_status_get_status_type(status));
}

gboolean
purple_status_is_active(PurpleStatus *status)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), FALSE);

	return status->active;
}

gboolean
purple_status_is_online(PurpleStatus *status)
{
	PurpleStatusPrimitive primitive;

	g_return_val_if_fail(PURPLE_IS_STATUS(status), FALSE);

	primitive = purple_status_type_get_primitive(purple_status_get_status_type(status));

	return (primitive != PURPLE_STATUS_UNSET &&
			primitive != PURPLE_STATUS_OFFLINE);
}

GValue *
purple_status_get_attr_value(PurpleStatus *status, const char *id)
{
	g_return_val_if_fail(PURPLE_IS_STATUS(status), NULL);
	g_return_val_if_fail(id   != NULL, NULL);

	return (GValue *)g_hash_table_lookup(status->attr_values, id);
}

gboolean
purple_status_get_attr_boolean(PurpleStatus *status, const char *id)
{
	const GValue *value;

	g_return_val_if_fail(PURPLE_IS_STATUS(status), FALSE);
	g_return_val_if_fail(id != NULL, FALSE);

	if ((value = purple_status_get_attr_value(status, id)) == NULL)
		return FALSE;

	g_return_val_if_fail(G_VALUE_TYPE(value) == G_TYPE_BOOLEAN, FALSE);

	return g_value_get_boolean(value);
}

int
purple_status_get_attr_int(PurpleStatus *status, const char *id)
{
	const GValue *value;

	g_return_val_if_fail(PURPLE_IS_STATUS(status), 0);
	g_return_val_if_fail(id != NULL, 0);

	if ((value = purple_status_get_attr_value(status, id)) == NULL)
		return 0;

	g_return_val_if_fail(G_VALUE_TYPE(value) == G_TYPE_INT, 0);

	return g_value_get_int(value);
}

const char *
purple_status_get_attr_string(PurpleStatus *status, const char *id)
{
	const GValue *value;

	g_return_val_if_fail(PURPLE_IS_STATUS(status), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	if ((value = purple_status_get_attr_value(status, id)) == NULL)
		return NULL;

	g_return_val_if_fail(G_VALUE_TYPE(value) == G_TYPE_STRING, NULL);

	return g_value_get_string(value);
}

gint
purple_status_compare(PurpleStatus *status1, PurpleStatus *status2)
{
	PurpleStatusType *type1, *type2;
	int score1 = 0, score2 = 0;

	if ((status1 == NULL && status2 == NULL) ||
			(status1 == status2))
	{
		return 0;
	}
	else if (status1 == NULL)
		return 1;
	else if (status2 == NULL)
		return -1;

	type1 = purple_status_get_status_type(status1);
	type2 = purple_status_get_status_type(status2);

	if (purple_status_is_active(status1))
		score1 = primitive_scores[purple_status_type_get_primitive(type1)];

	if (purple_status_is_active(status2))
		score2 = primitive_scores[purple_status_type_get_primitive(type2)];

	if (score1 > score2)
		return -1;
	else if (score1 < score2)
		return 1;

	return 0;
}


/**************************************************************************
* GBoxed code for PurpleStatusType
**************************************************************************/
static PurpleStatusType *
purple_status_type_ref(PurpleStatusType *status_type)
{
	g_return_val_if_fail(status_type != NULL, NULL);

	status_type->box_count++;

	return status_type;
}

static void
purple_status_type_unref(PurpleStatusType *status_type)
{
	g_return_if_fail(status_type != NULL);
	g_return_if_fail(status_type->box_count >= 0);

	if (!status_type->box_count--)
		purple_status_type_destroy(status_type);
}

GType
purple_status_type_get_type(void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static("PurpleStatusType",
				(GBoxedCopyFunc)purple_status_type_ref,
				(GBoxedFreeFunc)purple_status_type_unref);
	}

	return type;
}

/**************************************************************************
* GBoxed code for PurpleStatusAttribute
**************************************************************************/
static PurpleStatusAttribute *
purple_status_attribute_copy(PurpleStatusAttribute *status_attr)
{
	g_return_val_if_fail(status_attr != NULL, NULL);

	return purple_status_attribute_new(status_attr->id,
	                              status_attr->name,
	                              purple_value_dup(status_attr->value_type));
}

GType
purple_status_attribute_get_type(void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static("PurpleStatusAttribute",
				(GBoxedCopyFunc)purple_status_attribute_copy,
				(GBoxedFreeFunc)purple_status_attribute_destroy);
	}

	return type;
}

/**************************************************************************
* GBoxed code for PurpleMood
**************************************************************************/
static PurpleMood *
purple_mood_copy(PurpleMood *mood)
{
	PurpleMood *mood_copy;

	g_return_val_if_fail(mood != NULL, NULL);

	mood_copy = g_new(PurpleMood, 1);

	mood_copy->mood        = g_strdup(mood->mood);
	mood_copy->description = g_strdup(mood->description);

	return mood_copy;
}

static void
purple_mood_free(PurpleMood *mood)
{
	g_free((gchar *)mood->mood);
	g_free((gchar *)mood->description);

	g_free(mood);
}

GType
purple_mood_get_type(void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static("PurpleMood",
				(GBoxedCopyFunc)purple_mood_copy,
				(GBoxedFreeFunc)purple_mood_free);
	}

	return type;
}


/**************************************************************************
* GObject code
**************************************************************************/

/* Set method for GObject properties */
static void
purple_status_set_property(GObject *obj, guint param_id, const GValue *value,
		GParamSpec *pspec)
{
	PurpleStatus *status = PURPLE_STATUS(obj);

	switch (param_id) {
		case PROP_STATUS_TYPE:
			status->status_type = g_value_get_pointer(value);
			break;
		case PROP_PRESENCE:
			status->presence = g_value_get_object(value);
			break;
		case PROP_ACTIVE:
			purple_status_set_active(status, g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

/* Get method for GObject properties */
static void
purple_status_get_property(GObject *obj, guint param_id, GValue *value,
		GParamSpec *pspec)
{
	PurpleStatus *status = PURPLE_STATUS(obj);

	switch (param_id) {
		case PROP_STATUS_TYPE:
			g_value_set_pointer(value, purple_status_get_status_type(status));
			break;
		case PROP_PRESENCE:
			g_value_set_object(value, purple_status_get_presence(status));
			break;
		case PROP_ACTIVE:
			g_value_set_boolean(value, purple_status_is_active(status));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

/* GObject initialization function */
static void
purple_status_init(PurpleStatus *status)
{
	status->attr_values = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                                            (GDestroyNotify)purple_value_free);
}

/* Called when done constructing */
static void
purple_status_constructed(GObject *object)
{
	PurpleStatus *status = PURPLE_STATUS(object);
	GList *l;

	G_OBJECT_CLASS(purple_status_parent_class)->constructed(object);

	for (l = purple_status_type_get_attrs(status->status_type); l != NULL; l = l->next)
	{
		PurpleStatusAttribute *attr = (PurpleStatusAttribute *)l->data;
		GValue *value = purple_status_attribute_get_value(attr);
		GValue *new_value = purple_value_dup(value);

		g_hash_table_insert(status->attr_values,
							(char *)purple_status_attribute_get_id(attr),
							new_value);
	}
}

/*
 * GObject finalize function
 * TODO: If the PurpleStatus is in a PurplePresence, then
 *       remove it from the PurplePresence?
 */
static void
purple_status_finalize(GObject *obj)
{
	PurpleStatus *status = PURPLE_STATUS(obj);

	g_hash_table_destroy(status->attr_values);

	G_OBJECT_CLASS(purple_status_parent_class)->finalize(obj);
}

/* Class initializer function */
static void
purple_status_class_init(PurpleStatusClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_status_finalize;
	obj_class->constructed = purple_status_constructed;

	/* Setup properties */
	obj_class->get_property = purple_status_get_property;
	obj_class->set_property = purple_status_set_property;

	properties[PROP_STATUS_TYPE] = g_param_spec_pointer("status-type",
				"Status type",
				"The PurpleStatusType of the status.",
				G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				G_PARAM_STATIC_STRINGS);

	properties[PROP_PRESENCE] = g_param_spec_object("presence", "Presence",
				"The presence that the status belongs to.",
				PURPLE_TYPE_PRESENCE,
				G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				G_PARAM_STATIC_STRINGS);

	properties[PROP_ACTIVE] = g_param_spec_boolean("active", "Active",
				"Whether the status is active or not.", FALSE,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, PROP_LAST, properties);
}

PurpleStatus *
purple_status_new(PurpleStatusType *status_type, PurplePresence *presence)
{
	g_return_val_if_fail(status_type != NULL, NULL);
	g_return_val_if_fail(PURPLE_IS_PRESENCE(presence), NULL);

	return g_object_new(PURPLE_TYPE_STATUS,
			"status-type",  status_type,
			"presence",     presence,
			NULL);
}


/**************************************************************************
* Status subsystem
**************************************************************************/
static void
score_pref_changed_cb(const char *name, PurplePrefType type,
					  gconstpointer value, gpointer data)
{
	int index = GPOINTER_TO_INT(data);

	primitive_scores[index] = GPOINTER_TO_INT(value);
}

void *
purple_statuses_get_handle(void) {
	static int handle;

	return &handle;
}

void
purple_statuses_init(void)
{
	void *handle = purple_statuses_get_handle();

	purple_prefs_add_none("/purple/status");
	purple_prefs_add_none("/purple/status/scores");

	purple_prefs_add_int("/purple/status/scores/offline",
			primitive_scores[PURPLE_STATUS_OFFLINE]);
	purple_prefs_add_int("/purple/status/scores/available",
			primitive_scores[PURPLE_STATUS_AVAILABLE]);
	purple_prefs_add_int("/purple/status/scores/invisible",
			primitive_scores[PURPLE_STATUS_INVISIBLE]);
	purple_prefs_add_int("/purple/status/scores/away",
			primitive_scores[PURPLE_STATUS_AWAY]);
	purple_prefs_add_int("/purple/status/scores/extended_away",
			primitive_scores[PURPLE_STATUS_EXTENDED_AWAY]);
	purple_prefs_add_int("/purple/status/scores/idle",
			primitive_scores[SCORE_IDLE]);
	purple_prefs_add_int("/purple/status/scores/idle_time",
			primitive_scores[SCORE_IDLE_TIME]);
	purple_prefs_add_int("/purple/status/scores/offline_msg",
			primitive_scores[SCORE_OFFLINE_MESSAGE]);

	purple_prefs_connect_callback(handle, "/purple/status/scores/offline",
			score_pref_changed_cb,
			GINT_TO_POINTER(PURPLE_STATUS_OFFLINE));
	purple_prefs_connect_callback(handle, "/purple/status/scores/available",
			score_pref_changed_cb,
			GINT_TO_POINTER(PURPLE_STATUS_AVAILABLE));
	purple_prefs_connect_callback(handle, "/purple/status/scores/invisible",
			score_pref_changed_cb,
			GINT_TO_POINTER(PURPLE_STATUS_INVISIBLE));
	purple_prefs_connect_callback(handle, "/purple/status/scores/away",
			score_pref_changed_cb,
			GINT_TO_POINTER(PURPLE_STATUS_AWAY));
	purple_prefs_connect_callback(handle, "/purple/status/scores/extended_away",
			score_pref_changed_cb,
			GINT_TO_POINTER(PURPLE_STATUS_EXTENDED_AWAY));
	purple_prefs_connect_callback(handle, "/purple/status/scores/idle",
			score_pref_changed_cb,
			GINT_TO_POINTER(SCORE_IDLE));
	purple_prefs_connect_callback(handle, "/purple/status/scores/idle_time",
			score_pref_changed_cb,
			GINT_TO_POINTER(SCORE_IDLE_TIME));
	purple_prefs_connect_callback(handle, "/purple/status/scores/offline_msg",
			score_pref_changed_cb,
			GINT_TO_POINTER(SCORE_OFFLINE_MESSAGE));

	purple_prefs_trigger_callback("/purple/status/scores/offline");
	purple_prefs_trigger_callback("/purple/status/scores/available");
	purple_prefs_trigger_callback("/purple/status/scores/invisible");
	purple_prefs_trigger_callback("/purple/status/scores/away");
	purple_prefs_trigger_callback("/purple/status/scores/extended_away");
	purple_prefs_trigger_callback("/purple/status/scores/idle");
	purple_prefs_trigger_callback("/purple/status/scores/idle_time");
	purple_prefs_trigger_callback("/purple/status/scores/offline_msg");
}

void
purple_statuses_uninit(void)
{
	purple_prefs_disconnect_by_handle(purple_statuses_get_handle());
}

/**************************************************************************/
/* Helpers                                                                */
/**************************************************************************/
GHashTable *
purple_attrs_from_vargs(va_list args)
{
	GHashTable *attrs = g_hash_table_new(g_str_hash, g_str_equal);
	gchar *id;

	while ((id = va_arg(args, gchar *)) != NULL)
	{
		gpointer data = va_arg(args, gpointer);

		g_hash_table_insert(attrs, id, data);
	}

	return attrs;
}
