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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_STATUS_H
#define PURPLE_STATUS_H

#define PURPLE_TYPE_STATUS (purple_status_get_type())
typedef struct _PurpleStatus PurpleStatus;

#define PURPLE_TYPE_STATUS_TYPE (purple_status_type_get_type())

/**
 * PurpleStatusType:
 *
 * PurpleStatusType's are created by each protocol.  They outline the
 * available statuses of the protocol.  AIM, for example, supports
 * an available state with an optional available message, an away
 * state with a mandatory message, and an invisible state (which is
 * technically "independent" of the other two, but we'll get into
 * that later).  PurpleStatusTypes are very permanent.  They are
 * hardcoded in each protocol and will not change often.  And because
 * they are hardcoded, they do not need to be saved to any XML file.
 */
typedef struct _PurpleStatusType PurpleStatusType;

#define PURPLE_TYPE_STATUS_ATTRIBUTE (purple_status_attribute_get_type())

/**
 * PurpleStatus:
 *
 * A PurpleStatus can be thought of as an "instance" of a PurpleStatusType.
 * If you're familiar with object-oriented programming languages
 * then this should be immediately clear.  Say, for example, that
 * one of your AIM buddies has set himself as "away."  You have a
 * PurpleBuddy node for this person in your buddy list.  Purple wants
 * to mark this buddy as "away," so it creates a new PurpleStatus.
 * The PurpleStatus has its PurpleStatusType set to the "away" state
 * for the oscar protocol.  The PurpleStatus also contains the buddy's
 * away message.  PurpleStatuses are sometimes saved, depending on
 * the context.  The current PurpleStatuses associated with each of
 * your accounts are saved so that the next time you start Purple,
 * your accounts will be set to their last known statuses.  There
 * is also a list of saved statuses that are written to the
 * status.xml file.  Also, each PurpleStatus has a "saveable" boolean.
 * If "saveable" is set to FALSE then the status is NEVER saved.
 * All PurpleStatuses should be inside a PurplePresence.
 *
 * A PurpleStatus is either "independent" or "exclusive."
 * Independent statuses can be active or inactive and they don't
 * affect anything else.  However, you can only have one exclusive
 * status per PurplePresence.  If you activate one exclusive status,
 * then the previous exclusive status is automatically deactivated.
 *
 * A PurplePresence is like a collection of PurpleStatuses (plus some
 * other random info).
 *
 * See <link linkend="libpurple-purplepresence">Presence API</link>
 */
typedef struct _PurpleStatusAttribute PurpleStatusAttribute;

#define PURPLE_TYPE_MOOD (purple_mood_get_type())

/**
 * PurpleMood:
 * @mood: A string representing the mood.
 * @description: A short description of the mood.
 *
 * A structure to represent a mood.
 */
typedef struct {
	/*< public >*/
	const char *mood;
	const char *description;

	/*< private >*/
	gpointer padding[4];
} PurpleMood;

/**
 * PurpleStatusPrimitive:
 * @PURPLE_STATUS_UNSET: The status is not set
 * @PURPLE_STATUS_OFFLINE: The status is offline
 * @PURPLE_STATUS_AVAILABLE: The status is available
 * @PURPLE_STATUS_UNAVAILABLE: The status is unavailable
 * @PURPLE_STATUS_INVISIBLE: The stuatus is invisible
 * @PURPLE_STATUS_AWAY: The status is away
 * @PURPLE_STATUS_EXTENDED_AWAY: The status is extended away/do not disturb
 * @PURPLE_STATUS_MOBILE: The status is mobile
 * @PURPLE_STATUS_TUNE: The status includes a song title
 * @PURPLE_STATUS_MOOD: The status includes a mood
 * @PURPLE_STATUS_NUM_PRIMITIVES: The number of #PurpleStatusPrimitive<!-- -->s
 *
 * A primitive defining the basic structure of a status type.
 */
/*
 * If you add a value to this enum, make sure you update
 * the status_primitive_map and primitive_scores arrays in status.c.
 */
typedef enum
{
	PURPLE_STATUS_UNSET = 0,
	PURPLE_STATUS_OFFLINE,
	PURPLE_STATUS_AVAILABLE,
	PURPLE_STATUS_UNAVAILABLE,
	PURPLE_STATUS_INVISIBLE,
	PURPLE_STATUS_AWAY,
	PURPLE_STATUS_EXTENDED_AWAY,
	PURPLE_STATUS_MOBILE,
	PURPLE_STATUS_TUNE,
	PURPLE_STATUS_MOOD,

	PURPLE_STATUS_NUM_PRIMITIVES, /*< skip >*/
} PurpleStatusPrimitive;

#include "purplepresence.h"

/**
 * PURPLE_TUNE_ALBUM:
 *
 * A constant for a status attribute that represents the album of a song.
 */
#define PURPLE_TUNE_ALBUM "tune_album"

/**
 * PURPLE_TUNE_ARTIST:
 *
 * A constant for a status attribute that represents the artists of a song.
 */
#define PURPLE_TUNE_ARTIST "tune_artist"

/**
 * PURPLE_TUNE_COMMENT:
 *
 * A constant for a status attribute that represents the comment of a song.
 */
#define PURPLE_TUNE_COMMENT "tune_comment"

/**
 * PURPLE_TUNE_FULL:
 *
 * A constant for a status attribute that represents the full details of a song.
 */
#define PURPLE_TUNE_FULL "tune_full"

/**
 * PURPLE_TUNE_GENRE:
 *
 * A constant for a status attribute that represents the genre of a song.
 */
#define PURPLE_TUNE_GENRE "tune_genre"

/**
 * PURPLE_TUNE_TIME:
 *
 * A constant for a status attribute that represents the duration of a song.
 */
#define PURPLE_TUNE_TIME "tune_time"

/**
 * PURPLE_TUNE_TITLE:
 *
 * A constant for a status attribute that represents the title of a song.
 */
#define PURPLE_TUNE_TITLE "tune_title"

/**
 * PURPLE_TUNE_TRACK:
 *
 * A constant for a status attribute that represents the track number of a song.
 */
#define PURPLE_TUNE_TRACK "tune_track"

/**
 * PURPLE_TUNE_URL:
 *
 * A constant for a status attribute that represents the url of a song.
 */
#define PURPLE_TUNE_URL "tune_url"

/**
 * PURPLE_TUNE_YEAR:
 *
 * A constant for a status attribute that represents the release year of a song.
 */
#define PURPLE_TUNE_YEAR "tune_year"

#define PURPLE_MOOD_NAME	"mood"
#define PURPLE_MOOD_COMMENT	"moodtext"

G_BEGIN_DECLS

/**************************************************************************/
/* PurpleStatusPrimitive API                                              */
/**************************************************************************/

/**
 * purple_primitive_get_id_from_type:
 * @type: A primitive status type.
 *
 * Lookup the id of a primitive status type based on the type.  This
 * ID is a unique plain-text name of the status, without spaces.
 *
 * Returns: The unique ID for this type.
 */
const char *purple_primitive_get_id_from_type(PurpleStatusPrimitive type);

/**
 * purple_primitive_get_name_from_type:
 * @type: A primitive status type.
 *
 * Lookup the name of a primitive status type based on the type.  This
 * name is the plain-English name of the status type.  It is usually one
 * or two words.
 *
 * Returns: The name of this type, suitable for users to see.
 */
const char *purple_primitive_get_name_from_type(PurpleStatusPrimitive type);

/**
 * purple_primitive_get_type_from_id:
 * @id: The unique ID of a primitive status type.
 *
 * Lookup the value of a primitive status type based on the id.  The
 * ID is a unique plain-text name of the status, without spaces.
 *
 * Returns: The PurpleStatusPrimitive value.
 */
PurpleStatusPrimitive purple_primitive_get_type_from_id(const char *id);

/**************************************************************************/
/* PurpleStatusType API                                                   */
/**************************************************************************/

/**
 * purple_status_type_get_type:
 *
 * The standard _get_type function for #PurpleStatusType.
 *
 * Returns: The #GType for #PurpleStatusType.
 */
GType purple_status_type_get_type(void);

/**
 * purple_status_type_new_full:
 * @primitive: The primitive status type.
 * @id: The ID of the status type, or %NULL to use the id of the primitive
 *      status type.
 * @name: The name presented to the user, or %NULL to use the name of the
 *        primitive status type.
 * @saveable: %TRUE if the information set for this status by the user can be
 *            saved for future sessions.
 * @user_settable: %TRUE if this is a status the user can manually set.
 * @independent: %TRUE if this is an independent (non-exclusive) status type.
 *
 * Creates a new status type.
 *
 * Returns: (transfer full): A new status type.
 */
PurpleStatusType *purple_status_type_new_full(PurpleStatusPrimitive primitive,
										  const char *id, const char *name,
										  gboolean saveable,
										  gboolean user_settable,
										  gboolean independent);

/**
 * purple_status_type_new:
 * @primitive: The primitive status type.
 * @id: The ID of the status type, or %NULL to use the id of the primitive
 *      status type.
 * @name: The name presented to the user, or %NULL to use the name of the
 *        primitive status type.
 * @user_settable: %TRUE if this is a status the user can manually set.
 *
 * Creates a new status type with some default values (saveable and not
 * independent).
 *
 * Returns: (transfer full): A new status type.
 */
PurpleStatusType *purple_status_type_new(PurpleStatusPrimitive primitive,
									 const char *id, const char *name,
									 gboolean user_settable);

/**
 * purple_status_type_new_with_attrs:
 * @primitive: The primitive status type.
 * @id: The ID of the status type, or %NULL to use the id of the primitive
 *      status type.
 * @name: The name presented to the user, or %NULL to use the name of the
 *        primitive status type.
 * @saveable: %TRUE if the information set for this status by the user can be
 *            saved for future sessions.
 * @user_settable: %TRUE if this is a status the user can manually set.
 * @independent: %TRUE if this is an independent (non-exclusive) status type.
 * @attr_id: The ID of the first attribute.
 * @attr_name: The name of the first attribute.
 * @attr_value: The value type of the first attribute.
 * @...: Additional attribute information.
 *
 * Creates a new status type with attributes.
 *
 * Returns: (transfer full): A new status type.
 */
PurpleStatusType *purple_status_type_new_with_attrs(PurpleStatusPrimitive primitive,
												const char *id,
												const char *name,
												gboolean saveable,
												gboolean user_settable,
												gboolean independent,
												const char *attr_id,
												const char *attr_name,
												GValue *attr_value, ...) G_GNUC_NULL_TERMINATED;

/**
 * purple_status_type_new_with_attrsv: (rename-to purple_status_type_new_with_attrs):
 * @primitive: The primitive status type.
 * @id: The ID of the status type, or %NULL to use the id of the primitive
 *      status type.
 * @name: The name presented to the user, or %NULL to use the name of the
 *        primitive status type.
 * @saveable: %TRUE if the information set for this status by the user can be
 *            saved for future sessions.
 * @user_settable: %TRUE if this is a status the user can manually set.
 * @independent: %TRUE if this is an independent (non-exclusive) status type.
 * @n_attributes: The number of attributes.
 * @attr_id: (array length=n_attributes): The IDs of the attributes.
 * @attr_name: (array length=n_attributes): The names of the attributes.
 * @attr_value: (array length=n_attributes): The value types of the attributes.
 *
 * Creates a new status type with attributes.
 *
 * Returns: (transfer full): A new status type.
 *
 * Since: 3.0.0
 */
PurpleStatusType *purple_status_type_new_with_attrsv(PurpleStatusPrimitive primitive,
                                                     const gchar *id,
                                                     const gchar *name,
                                                     gboolean saveable,
                                                     gboolean user_settable,
                                                     gboolean independent,
                                                     guint n_attributes,
                                                     const gchar *attr_id[],
                                                     const gchar *attr_name[],
                                                     GValue *attr_value[]);

/**
 * purple_status_type_destroy:
 * @status_type: The status type to destroy.
 *
 * Destroys a status type.
 */
void purple_status_type_destroy(PurpleStatusType *status_type);

/**
 * purple_status_type_get_primitive:
 * @status_type: The status type.
 *
 * Returns the primitive type of a status type.
 *
 * Returns: The primitive type of the status type.
 */
PurpleStatusPrimitive purple_status_type_get_primitive(
	const PurpleStatusType *status_type);

/**
 * purple_status_type_get_id:
 * @status_type: The status type.
 *
 * Returns the ID of a status type.
 *
 * Returns: The ID of the status type.
 */
const char *purple_status_type_get_id(const PurpleStatusType *status_type);

/**
 * purple_status_type_get_name:
 * @status_type: The status type.
 *
 * Returns the name of a status type.
 *
 * Returns: The name of the status type.
 */
const char *purple_status_type_get_name(const PurpleStatusType *status_type);

/**
 * purple_status_type_is_saveable:
 * @status_type: The status type.
 *
 * Returns whether or not the status type is saveable.
 *
 * Returns: TRUE if user-defined statuses based off this type are saveable.
 *         FALSE otherwise.
 */
gboolean purple_status_type_is_saveable(const PurpleStatusType *status_type);

/**
 * purple_status_type_is_user_settable:
 * @status_type: The status type.
 *
 * Returns whether or not the status type can be set or modified by the
 * user.
 *
 * Returns: TRUE if the status type can be set or modified by the user.
 *         FALSE if it's a protocol-set setting.
 */
gboolean purple_status_type_is_user_settable(const PurpleStatusType *status_type);

/**
 * purple_status_type_is_independent:
 * @status_type: The status type.
 *
 * Returns whether or not the status type is independent.
 *
 * Independent status types are non-exclusive. If other status types on
 * the same hierarchy level are set, this one will not be affected.
 *
 * Returns: TRUE if the status type is independent, or FALSE otherwise.
 */
gboolean purple_status_type_is_independent(const PurpleStatusType *status_type);

/**
 * purple_status_type_is_exclusive:
 * @status_type: The status type.
 *
 * Returns whether the status type is exclusive.
 *
 * Returns: TRUE if the status type is exclusive, FALSE otherwise.
 */
gboolean purple_status_type_is_exclusive(const PurpleStatusType *status_type);

/**
 * purple_status_type_is_available:
 * @status_type: The status type.
 *
 * Returns whether or not a status type is available.
 *
 * Available status types are online and possibly invisible, but not away.
 *
 * Returns: TRUE if the status is available, or FALSE otherwise.
 */
gboolean purple_status_type_is_available(const PurpleStatusType *status_type);

/**
 * purple_status_type_get_attr:
 * @status_type: The status type containing the attribute.
 * @id:          The ID of the desired attribute.
 *
 * Returns the attribute with the specified ID.
 *
 * Returns: The attribute, if found. NULL otherwise.
 */
PurpleStatusAttribute *purple_status_type_get_attr(const PurpleStatusType *status_type,
										  const char *id);

/**
 * purple_status_type_get_attrs:
 * @status_type: The status type.
 *
 * Returns a list of all attributes in a status type.
 *
 * Returns: (element-type PurpleStatusAttribute) (transfer none): The list of attributes.
 */
GList *purple_status_type_get_attrs(const PurpleStatusType *status_type);

/**
 * purple_status_type_find_with_id:
 * @status_types: (element-type PurpleStatus) (transfer none): A list of status
 *                types.  Often account->status_types.
 * @id: The unique ID of the status type you wish to find.
 *
 * Find the PurpleStatusType with the given id.
 *
 * Returns: The status type with the given ID, or NULL if one could
 *         not be found.
 */
const PurpleStatusType *purple_status_type_find_with_id(GList *status_types,
													const char *id);

/**************************************************************************/
/* PurpleStatusAttribute API                                              */
/**************************************************************************/

/**
 * purple_status_attribute_get_type:
 *
 * The standard _get_type function for #PurpleStatusAttribute.
 *
 * Returns: The #GType for the #PurpleStatusAttribute boxed structure.
 */
GType purple_status_attribute_get_type(void);

/**
 * purple_status_attribute_new:
 * @id:         The ID of the attribute.
 * @name:       The name presented to the user.
 * @value_type: The type of data contained in the attribute.
 *
 * Creates a new status attribute.
 *
 * Returns: A new status attribute.
 */
PurpleStatusAttribute *purple_status_attribute_new(const char *id, const char *name,
									 GValue *value_type);

/**
 * purple_status_attribute_destroy:
 * @attr: The status attribute to destroy.
 *
 * Destroys a status attribute.
 */
void purple_status_attribute_destroy(PurpleStatusAttribute *attr);

/**
 * purple_status_attribute_get_id:
 * @attr: The status attribute.
 *
 * Returns the ID of a status attribute.
 *
 * Returns: The status attribute's ID.
 */
const char *purple_status_attribute_get_id(const PurpleStatusAttribute *attr);

/**
 * purple_status_attribute_get_name:
 * @attr: The status attribute.
 *
 * Returns the name of a status attribute.
 *
 * Returns: The status attribute's name.
 */
const char *purple_status_attribute_get_name(const PurpleStatusAttribute *attr);

/**
 * purple_status_attribute_get_value:
 * @attr: The status attribute.
 *
 * Returns the value of a status attribute.
 *
 * Returns: The status attribute's value.
 */
GValue *purple_status_attribute_get_value(const PurpleStatusAttribute *attr);

/**************************************************************************/
/* PurpleMood API                                                         */
/**************************************************************************/

/**
 * purple_mood_get_type:
 *
 * The standard _get_type function for #PurpleMood.
 *
 * Returns: The #GType for the #PurpleMood boxed structure.
 */
GType purple_mood_get_type(void);

/**************************************************************************/
/* PurpleStatus API                                                       */
/**************************************************************************/

/**
 * purple_status_get_type:
 *
 * The standard _get_type function for #PurpleStatus.
 *
 * Returns: The #GType for #PurpleStatus.
 */
G_DECLARE_FINAL_TYPE(PurpleStatus, purple_status, PURPLE, STATUS, GObject)

/**
 * purple_status_new:
 * @status_type: The type of status.
 * @presence:    The parent presence.
 *
 * Creates a new status.
 *
 * Returns: The new status.
 */
PurpleStatus *purple_status_new(PurpleStatusType *status_type,
							PurplePresence *presence);

/**
 * purple_status_set_active:
 * @status: The status.
 * @active: The active state.
 *
 * Sets whether or not a status is active.
 *
 * This should only be called by the account, conversation, and buddy APIs.
 */
void purple_status_set_active(PurpleStatus *status, gboolean active);

/**
 * purple_status_set_active_with_attrs:
 * @status: The status.
 * @active: The active state.
 * @args:   A list of attributes to set on the status.  This list is
 *               composed of key/value pairs, where each key is a valid
 *               attribute name for this PurpleStatusType.  The list should
 *               be NULL terminated.
 *
 * Sets whether or not a status is active.
 *
 * This should only be called by the account, conversation, and buddy APIs.
 */
void purple_status_set_active_with_attrs(PurpleStatus *status, gboolean active,
									   va_list args);

/**
 * purple_status_set_active_with_attrs_dict: (rename-to purple_status_set_active_with_attrs)
 * @status: The status.
 * @active: The active state.
 * @attrs: (element-type utf8 gpointer): A hash table of attributes to set on
 *         the status.  This hash table's keys are valid attribute names for
 *         this PurpleStatusType.  The hash table is not modified or freed by
 *         this function.
 *
 * Sets whether or not a status is active.
 *
 * This should only be called by the account, conversation, and buddy APIs.
 *
 * Since: 3.0.0
 */
void purple_status_set_active_with_attrs_dict(PurpleStatus *status, gboolean active, GHashTable *attrs);

/**
 * purple_status_get_status_type:
 * @status: The status.
 *
 * Returns the status's type.
 *
 * Returns: The status's type.
 */
PurpleStatusType *purple_status_get_status_type(PurpleStatus *status);

/**
 * purple_status_get_presence:
 * @status: The status.
 *
 * Returns the status's presence.
 *
 * Returns: (transfer none): The status's presence.
 */
PurplePresence *purple_status_get_presence(PurpleStatus *status);

/**
 * purple_status_get_id:
 * @status: The status.
 *
 * Returns the status's type ID.
 *
 * This is a convenience method for
 * purple_status_type_get_id(purple_status_get_status_type(status)).
 *
 * Returns: The status's ID.
 */
const char *purple_status_get_id(PurpleStatus *status);

/**
 * purple_status_get_name:
 * @status: The status.
 *
 * Returns the status's name.
 *
 * This is a convenience method for
 * purple_status_type_get_name(purple_status_get_status_type(status)).
 *
 * Returns: The status's name.
 */
const char *purple_status_get_name(PurpleStatus *status);

/**
 * purple_status_is_independent:
 * @status: The status.
 *
 * Returns whether or not a status is independent.
 *
 * This is a convenience method for
 * purple_status_type_is_independent(purple_status_get_status_type(status)).
 *
 * Returns: TRUE if the status is independent, or FALSE otherwise.
 */
gboolean purple_status_is_independent(PurpleStatus *status);

/**
 * purple_status_is_exclusive:
 * @status: The status.
 *
 * Returns whether or not a status is exclusive.
 *
 * This is a convenience method for
 * purple_status_type_is_exclusive(purple_status_get_status_type(status)).
 *
 * Returns: TRUE if the status is exclusive, FALSE otherwise.
 */
gboolean purple_status_is_exclusive(PurpleStatus *status);

/**
 * purple_status_is_available:
 * @status: The status.
 *
 * Returns whether or not a status is available.
 *
 * Available statuses are online and possibly invisible, but not away or idle.
 *
 * This is a convenience method for
 * purple_status_type_is_available(purple_status_get_status_type(status)).
 *
 * Returns: TRUE if the status is available, or FALSE otherwise.
 */
gboolean purple_status_is_available(PurpleStatus *status);

/**
 * purple_status_is_active:
 * @status: The status.
 *
 * Returns the active state of a status.
 *
 * Returns: The active state of the status.
 */
gboolean purple_status_is_active(PurpleStatus *status);

/**
 * purple_status_is_online:
 * @status: The status.
 *
 * Returns whether or not a status is considered 'online'
 *
 * Returns: TRUE if the status is considered online, FALSE otherwise
 */
gboolean purple_status_is_online(PurpleStatus *status);

/**
 * purple_status_get_attr_value:
 * @status: The status.
 * @id:     The attribute ID.
 *
 * Returns the value of an attribute in a status with the specified ID.
 *
 * Returns: The value of the attribute.
 */
GValue *purple_status_get_attr_value(PurpleStatus *status,
									 const char *id);

/**
 * purple_status_get_attr_boolean:
 * @status: The status.
 * @id:     The attribute ID.
 *
 * Returns the boolean value of an attribute in a status with the specified ID.
 *
 * Returns: The boolean value of the attribute.
 */
gboolean purple_status_get_attr_boolean(PurpleStatus *status,
									    const char *id);

/**
 * purple_status_get_attr_int:
 * @status: The status.
 * @id:     The attribute ID.
 *
 * Returns the integer value of an attribute in a status with the specified ID.
 *
 * Returns: The integer value of the attribute.
 */
int purple_status_get_attr_int(PurpleStatus *status, const char *id);

/**
 * purple_status_get_attr_string:
 * @status: The status.
 * @id:     The attribute ID.
 *
 * Returns the string value of an attribute in a status with the specified ID.
 *
 * Returns: The string value of the attribute.
 */
const char *purple_status_get_attr_string(PurpleStatus *status, const char *id);

/**
 * purple_status_compare:
 * @status1: The first status.
 * @status2: The second status.
 *
 * Compares two statuses for availability.
 *
 * Returns: -1 if @status1 is more available than @status2.
 *           0 if @status1 is equal to @status2.
 *           1 if @status2 is more available than @status1.
 */
gint purple_status_compare(PurpleStatus *status1, PurpleStatus *status2);

/**************************************************************************/
/* Statuses subsystem                                                     */
/**************************************************************************/

/**
 * purple_statuses_get_handle:
 *
 * Get the handle for the status subsystem.
 *
 * Returns: the handle to the status subsystem
 */
void *purple_statuses_get_handle(void);

/**
 * purple_statuses_init:
 *
 * Initializes the status subsystem.
 */
void purple_statuses_init(void);

/**
 * purple_statuses_uninit:
 *
 * Uninitializes the status subsystem.
 */
void purple_statuses_uninit(void);

/**************************************************************************/
/* Helpers                                                                */
/**************************************************************************/

/**
 * purple_attrs_from_vargs:
 * @args:   A list of attributes to parse.  This list is
 *               composed of key/value pairs, where each key is a valid
 *               attribute name for this PurpleStatusType.  The list should
 *               be NULL terminated.
 *
 * Returns a hash table of attributes constructed from args.
 *
 * Returns: (element-type utf8 gpointer) (transfer container): The hash table of attributes.
 *
 * Since: 3.0.0
 */
GHashTable *purple_attrs_from_vargs(va_list args);

G_END_DECLS

#endif /* PURPLE_STATUS_H */
