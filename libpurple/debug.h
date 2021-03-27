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

#ifndef PURPLE_DEBUG_H
#define PURPLE_DEBUG_H

/**
 * SECTION:debug
 * @section_id: libpurple-debug
 * @short_description: Developer Logging API
 * @title: Debug API
 */

#include <glib.h>
#include <glib-object.h>

#include <stdarg.h>

G_BEGIN_DECLS

/**
 * PURPLE_TYPE_DEBUG_UI:
 *
 * The standard _get_type macro for #PurpleDebugUi.
 */
#define PURPLE_TYPE_DEBUG_UI (purple_debug_ui_get_type())
G_DECLARE_INTERFACE(PurpleDebugUi, purple_debug_ui, PURPLE, DEBUG_UI, GObject)

/**
 * PurpleDebugLevel:
 * @PURPLE_DEBUG_ALL: All debug levels.
 * @PURPLE_DEBUG_MISC: General chatter.
 * @PURPLE_DEBUG_INFO: General operation Information.
 * @PURPLE_DEBUG_WARNING: Warnings.
 * @PURPLE_DEBUG_ERROR: Errors.
 * @PURPLE_DEBUG_FATAL: Fatal errors.
 *
 * Available debug levels.
 */
typedef enum {
	PURPLE_DEBUG_ALL = 0,
	PURPLE_DEBUG_MISC,
	PURPLE_DEBUG_INFO,
	PURPLE_DEBUG_WARNING,
	PURPLE_DEBUG_ERROR,
	PURPLE_DEBUG_FATAL

} PurpleDebugLevel;

/**
 * PurpleDebugUiInterface:
 * @print: Called to output a debug string to the UI.
 * @is_enabled: Returns if debug printing is enabled in the UI for a @level and
 *              @category.
 *
 * Debug UI operations.
 */
struct _PurpleDebugUiInterface {
	/*< private >*/
	GTypeInterface parent_iface;

	/*< public >*/
	void (*print)(PurpleDebugUi *self, PurpleDebugLevel level, const gchar *category, const gchar *arg_s);
	gboolean (*is_enabled)(PurpleDebugUi *self, PurpleDebugLevel level, const gchar *category);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * purple_debug:
 * @level: The debug level.
 * @category: The category (or %NULL).
 * @format: The format string.
 * @...:  The parameters to insert into the format string.
 *
 * Outputs debug information.
 */
void purple_debug(PurpleDebugLevel level, const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(3, 4);

/**
 * purple_debug_misc:
 * @category: The category or %NULL.
 * @format: The format string.
 * @...:  The parameters to insert into the format string.
 *
 * Outputs misc. level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_MISC as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_misc(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_info:
 * @category: The category (or %NULL).
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs info level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_INFO as
 * the level.
 *
 * See purple_debug().
 */
void purple_debug_info(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_warning:
 * @category: The category or %NULL.
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs warning level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_WARNING as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_warning(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_error:
 * @category: The category or %NULL.
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs error level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_ERROR as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_error(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_fatal:
 * @category: The category (or %NULL).
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs fatal error level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_ERROR as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_fatal(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_set_enabled:
 * @enabled: %TRUE to enable debug output or %FALSE to disable it.
 *
 * Enable or disable printing debug output to the console.
 */
void purple_debug_set_enabled(gboolean enabled);

/**
 * purple_debug_is_enabled:
 *
 * Check if console debug output is enabled.
 *
 * Returns: %TRUE if debugging is enabled, %FALSE if it is not.
 */
gboolean purple_debug_is_enabled(void);

/**
 * purple_debug_set_verbose:
 * @verbose: %TRUE to enable verbose debugging or %FALSE to disable it.
 *
 * Enable or disable verbose debugging.  This ordinarily should only be called
 * by purple_debug_init(), but there are cases where this can be useful for
 * plugins.
 */
void purple_debug_set_verbose(gboolean verbose);

/**
 * purple_debug_is_verbose:
 *
 * Check if verbose logging is enabled.
 *
 * Returns: %TRUE if verbose debugging is enabled, %FALSE if it is not.
 */
gboolean purple_debug_is_verbose(void);

/**
 * purple_debug_set_unsafe:
 * @unsafe: %TRUE to enable debug logging of messages that could potentially
 *          contain passwords and other sensitive information. %FALSE to
 *          disable it.
 *
 * Enable or disable unsafe debugging. This ordinarily should only be called by
 * purple_debug_init(), but there are cases where this can be useful for
 * plugins.
 */
void purple_debug_set_unsafe(gboolean unsafe);

/**
 * purple_debug_is_unsafe:
 *
 * Check if unsafe debugging is enabled. Defaults to %FALSE.
 *
 * Returns: %TRUE if the debug logging of all messages is enabled, %FALSE if
 *          messages that could potentially contain passwords and other
 *          sensitive information are not logged.
 */
gboolean purple_debug_is_unsafe(void);

/**
 * purple_debug_set_colored:
 * @colored: %TRUE to enable colored output, %FALSE to disable it.
 *
 * Enable or disable colored output for bash console.
 */
void purple_debug_set_colored(gboolean colored);

/******************************************************************************
 * UI Registration Functions
 *****************************************************************************/

/**
 * purple_debug_set_ui:
 * @ui: The UI operations structure.
 *
 * Sets the UI operations structure to be used when outputting debug
 * information.
 *
 * Since: 3.0.0
 */
void purple_debug_set_ui(PurpleDebugUi *ui);

/**
 * purple_debug_get_ui:
 *
 * Get the #PurpleDebugUi instance used for outputting debug information.
 *
 * Returns: (transfer none): The #PurpleDebugUi instance in use.
 *
 * Since: 3.0.0
 */
PurpleDebugUi *purple_debug_get_ui(void);

/**
 * purple_debug_ui_is_enabled:
 * @ui: The #PurpleDebugUi instance.
 * @level: The #PurpleLogLevel.
 * @category: An optional category.
 *
 * Checks if the ui should output messages at the given level and optional
 * category.
 *
 * Typically this function will not need to be called outside of libpurple.
 *
 * Returns: %TRUE if the given level and category will be output by @ui, %FALSE
 *          otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_debug_ui_is_enabled(PurpleDebugUi *ui, PurpleDebugLevel level, const gchar *category);

/**
 * purple_debug_ui_print:
 * @ui: The #PurpleDebugUi instance.
 * @level: The #PurpleDebugLevel.
 * @category: An optional category.
 * @arg_s: The debug string to output.
 *
 * Outputs @arg_s via @ui with the given @level and optional @category.
 *
 * Since: 3.0.0
 */
void purple_debug_ui_print(PurpleDebugUi *ui, PurpleDebugLevel level, const gchar *category, const gchar *arg_s);

/******************************************************************************
 * Debug Subsystem
 *****************************************************************************/

/**
 * purple_debug_init:
 *
 * Initializes the debug subsystem.
 */
void purple_debug_init(void);

G_END_DECLS

#endif /* PURPLE_DEBUG_H */
