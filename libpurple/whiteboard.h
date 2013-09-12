/**
 * @file whiteboard.h The PurpleWhiteboard core object
 */
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

#ifndef _PURPLE_WHITEBOARD_H_
#define _PURPLE_WHITEBOARD_H_

#define PURPLE_TYPE_WHITEBOARD             (purple_whiteboard_get_type())
#define PURPLE_WHITEBOARD(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), PURPLE_TYPE_WHITEBOARD, PurpleWhiteboard))
#define PURPLE_WHITEBOARD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), PURPLE_TYPE_WHITEBOARD, PurpleWhiteboardClass))
#define PURPLE_IS_WHITEBOARD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), PURPLE_TYPE_WHITEBOARD))
#define PURPLE_IS_WHITEBOARD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), PURPLE_TYPE_WHITEBOARD))
#define PURPLE_WHITEBOARD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PURPLE_TYPE_WHITEBOARD, PurpleWhiteboardClass))

/** @copydoc _PurpleWhiteboard */
typedef struct _PurpleWhiteboard PurpleWhiteboard;
/** @copydoc _PurpleWhiteboardClass */
typedef struct _PurpleWhiteboardClass PurpleWhiteboardClass;

/**
 * Whiteboard protocol operations
 */
typedef struct _PurpleWhiteboardOps PurpleWhiteboardOps;

#include "account.h"

/**
 * The PurpleWhiteboard UI Operations
 */
typedef struct _PurpleWhiteboardUiOps
{
	void (*create)(PurpleWhiteboard *wb);                                /**< create function */
	void (*destroy)(PurpleWhiteboard *wb);                               /**< destory function */
	void (*set_dimensions)(PurpleWhiteboard *wb, int width, int height); /**< set_dimensions function */
	void (*set_brush) (PurpleWhiteboard *wb, int size, int color);       /**< set the size and color of the brush */
	void (*draw_point)(PurpleWhiteboard *wb, int x, int y,
	                   int color, int size);                             /**< draw_point function */
	void (*draw_line)(PurpleWhiteboard *wb, int x1, int y1,
	                  int x2, int y2,
	                  int color, int size);                              /**< draw_line function */
	void (*clear)(PurpleWhiteboard *wb);                                 /**< clear function */

	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
} PurpleWhiteboardUiOps;

/**
 * PurpleWhiteboard Protocol Operations
 */
struct _PurpleWhiteboardOps
{
	void (*start)(PurpleWhiteboard *wb);                                   /**< start function */
	void (*end)(PurpleWhiteboard *wb);                                     /**< end function */
	void (*get_dimensions)(const PurpleWhiteboard *wb, int *width, int *height); /**< get_dimensions function */
	void (*set_dimensions)(PurpleWhiteboard *wb, int width, int height);   /**< set_dimensions function */
	void (*get_brush) (const PurpleWhiteboard *wb, int *size, int *color); /**< get the brush size and color */
	void (*set_brush) (PurpleWhiteboard *wb, int size, int color);         /**< set the brush size and color */
	void (*send_draw_list)(PurpleWhiteboard *wb, GList *draw_list);        /**< send_draw_list function */
	void (*clear)(PurpleWhiteboard *wb);                                   /**< clear function */

	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

/**
 * A PurpleWhiteboard
 */
struct _PurpleWhiteboard
{
	/*< private >*/
	GObject gparent;

	/** The UI data associated with this whiteboard. This is a convenience
	 *  field provided to the UIs -- it is not used by the libpurple core.
	 */
	gpointer ui_data;
};

/** Base class for all #PurpleWhiteboard's */
struct _PurpleWhiteboardClass {
	/*< private >*/
	GObjectClass parent_class;

	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

G_BEGIN_DECLS

/******************************************************************************/
/** @name PurpleWhiteboard API                                                  */
/******************************************************************************/
/*@{*/

/**
 * Returns the GType for the PurpleWhiteboard object.
 */
GType purple_whiteboard_get_type(void);

/**
 * Sets the UI operations
 *
 * @param ops The UI operations to set
 */
void purple_whiteboard_set_ui_ops(PurpleWhiteboardUiOps *ops);

/**
 * Sets the protocol operations for a whiteboard
 *
 * @param wb  The whiteboard for which to set the protocol operations
 * @param ops The protocol operations to set
 */
void purple_whiteboard_set_protocol_ops(PurpleWhiteboard *wb, PurpleWhiteboardOps *ops);

/**
 * Creates a new whiteboard
 *
 * @param account The account.
 * @param who     Who you're drawing with.
 *
 * @return The new whiteboard
 */
PurpleWhiteboard *purple_whiteboard_new(PurpleAccount *account, const char *who);

/**
 * Returns the whiteboard's account.
 *
 * @param wb		The whiteboard.
 *
 * @return The whiteboard's account.
 */
PurpleAccount *purple_whiteboard_get_account(const PurpleWhiteboard *wb);

/**
 * Return who you're drawing with.
 *
 * @param wb		The whiteboard
 *
 * @return Who you're drawing with.
 */
const char *purple_whiteboard_get_who(const PurpleWhiteboard *wb);

/**
 * Starts a whiteboard
 *
 * @param wb The whiteboard.
 */
void purple_whiteboard_start(PurpleWhiteboard *wb);

/**
 * Finds a whiteboard from an account and user.
 *
 * @param account The account.
 * @param who     The user.
 *
 * @return The whiteboard if found, otherwise @c NULL.
 */
PurpleWhiteboard *purple_whiteboard_get_session(const PurpleAccount *account, const char *who);

/**
 * Destorys a drawing list for a whiteboard
 *
 * @param draw_list The drawing list.
 */
void purple_whiteboard_draw_list_destroy(GList *draw_list);

/**
 * Gets the dimension of a whiteboard.
 *
 * @param wb		The whiteboard.
 * @param width		The width to be set.
 * @param height	The height to be set.
 *
 * @return TRUE if the values of width and height were set.
 */
gboolean purple_whiteboard_get_dimensions(const PurpleWhiteboard *wb, int *width, int *height);

/**
 * Sets the dimensions for a whiteboard.
 *
 * @param wb     The whiteboard.
 * @param width  The width.
 * @param height The height.
 */
void purple_whiteboard_set_dimensions(PurpleWhiteboard *wb, int width, int height);

/**
 * Draws a point on a whiteboard.
 *
 * @param wb    The whiteboard.
 * @param x     The x coordinate.
 * @param y     The y coordinate.
 * @param color The color to use.
 * @param size  The brush size.
 */
void purple_whiteboard_draw_point(PurpleWhiteboard *wb, int x, int y, int color, int size);

/**
 * Send a list of points to draw to the buddy.
 *
 * @param wb	The whiteboard
 * @param list	A GList of points
 */
void purple_whiteboard_send_draw_list(PurpleWhiteboard *wb, GList *list);

/**
 * Draws a line on a whiteboard
 *
 * @param wb    The whiteboard.
 * @param x1    The top-left x coordinate.
 * @param y1    The top-left y coordinate.
 * @param x2    The bottom-right x coordinate.
 * @param y2    The bottom-right y coordinate.
 * @param color The color to use.
 * @param size  The brush size.
 */
void purple_whiteboard_draw_line(PurpleWhiteboard *wb, int x1, int y1, int x2, int y2, int color, int size);

/**
 * Clears a whiteboard
 *
 * @param wb The whiteboard.
 */
void purple_whiteboard_clear(PurpleWhiteboard *wb);

/**
 * Sends a request to the buddy to clear the whiteboard.
 *
 * @param wb The whiteboard
 */
void purple_whiteboard_send_clear(PurpleWhiteboard *wb);

/**
 * Sends a request to change the size and color of the brush.
 *
 * @param wb	The whiteboard
 * @param size	The size of the brush
 * @param color	The color of the brush
 */
void purple_whiteboard_send_brush(PurpleWhiteboard *wb, int size, int color);

/**
 * Gets the size and color of the brush.
 *
 * @param wb	The whiteboard
 * @param size	The size of the brush
 * @param color	The color of the brush
 *
 * @return	TRUE if the size and color were set.
 */
gboolean purple_whiteboard_get_brush(const PurpleWhiteboard *wb, int *size, int *color);

/**
 * Sets the size and color of the brush.
 *
 * @param wb	The whiteboard
 * @param size	The size of the brush
 * @param color	The color of the brush
 */
void purple_whiteboard_set_brush(PurpleWhiteboard *wb, int size, int color);

/**
 * Return the drawing list.
 *
 * @param wb			The whiteboard.
 *
 * @return The drawing list
 */
GList *purple_whiteboard_get_draw_list(const PurpleWhiteboard *wb);

/**
 * Set the drawing list.
 *
 * @param wb			The whiteboard
 * @param draw_list		The drawing list.
 */
void purple_whiteboard_set_draw_list(PurpleWhiteboard *wb, GList* draw_list);

/**
 * Set the UI data associated with this whiteboard.
 *
 * @param wb			The whiteboard.
 * @param ui_data		A pointer to associate with this whiteboard.
 */
void purple_whiteboard_set_ui_data(PurpleWhiteboard *wb, gpointer ui_data);

/**
 * Get the UI data associated with this whiteboard.
 *
 * @param wb			The whiteboard..
 *
 * @return The UI data associated with this whiteboard.  This is a
 *         convenience field provided to the UIs--it is not
 *         used by the libpurple core.
 */
gpointer purple_whiteboard_get_ui_data(const PurpleWhiteboard *wb);

/*@}*/

G_END_DECLS

#endif /* _PURPLE_WHITEBOARD_H_ */
