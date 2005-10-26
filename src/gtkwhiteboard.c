/*
 * gaim
 *
 * Gaim is the legal property of its developers, whose names are too numerous
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>

#include "blist.h"

#include "gtkwhiteboard.h"

/******************************************************************************
 * Prototypes
 *****************************************************************************/
static void			gaim_gtk_whiteboard_create( GaimWhiteboard *wb );

static void			gaim_gtk_whiteboard_destroy( GaimWhiteboard *wb );
static void			gaim_gtk_whiteboard_exit( GtkWidget *widget, gpointer data );

/*static void			gaim_gtkwhiteboard_button_start_press( GtkButton *button, gpointer data ); */

static gboolean		gaim_gtk_whiteboard_configure_event( GtkWidget *widget, GdkEventConfigure *event, gpointer data );
static gboolean		gaim_gtk_whiteboard_expose_event( GtkWidget *widget, GdkEventExpose *event, gpointer data );

static gboolean		gaim_gtk_whiteboard_brush_down( GtkWidget *widget, GdkEventButton *event, gpointer data );
static gboolean		gaim_gtk_whiteboard_brush_motion( GtkWidget *widget, GdkEventMotion *event, gpointer data );
static gboolean		gaim_gtk_whiteboard_brush_up( GtkWidget *widget, GdkEventButton *event, gpointer data );

static void			gaim_gtk_whiteboard_draw_brush_point( GaimWhiteboard *wb,
							      int x, int y, int color, int size );
static void			gaim_gtk_whiteboard_draw_brush_line( GaimWhiteboard *wb,
							     int x0, int y0, int x1, int y1, int color, int size );

static void			gaim_gtk_whiteboard_set_dimensions( GaimWhiteboard *wb, int width, int height );
static void			gaim_gtk_whiteboard_clear( GaimWhiteboard *wb );

static void			gaim_gtk_whiteboard_button_clear_press( GtkWidget *widget, gpointer data );
static void			gaim_gtk_whiteboard_button_save_press( GtkWidget *widget, gpointer data );

static void			gaim_gtk_whiteboard_set_canvas_as_icon( GaimGtkWhiteboard *gtkwb );

static void			gaim_gtk_whiteboard_rgb24_to_rgb48( int color_rgb, GdkColor *color );

/******************************************************************************
 * Globals
 *****************************************************************************/
/*
GList				*buttonList		= NULL;
GdkColor			DefaultColor[PALETTE_NUM_COLORS];
*/

static gboolean			LocalShutdownRequest;

static int			LastX;			/* Tracks last position of the mouse when drawing */
static int			LastY;
static int			MotionCount;		/* Tracks how many brush motions made */
static int			BrushState		= BRUSH_STATE_UP;

static GaimWhiteboardUiOps 	ui_ops			=
{
	gaim_gtk_whiteboard_create,
	gaim_gtk_whiteboard_destroy,
	gaim_gtk_whiteboard_set_dimensions,
	gaim_gtk_whiteboard_draw_brush_point,
	gaim_gtk_whiteboard_draw_brush_line,
	gaim_gtk_whiteboard_clear
};

/******************************************************************************
 * API
 *****************************************************************************/
GaimWhiteboardUiOps *gaim_gtk_whiteboard_get_ui_ops( void )
{
	return( &ui_ops );
}

void gaim_gtk_whiteboard_create( GaimWhiteboard *wb )
{
	int i;

	GtkWidget *window;
	GtkWidget *drawing_area;

	GtkWidget *hbox_palette;
	GtkWidget *vbox_palette_above_canvas_and_controls;
	GtkWidget *hbox_canvas_and_controls;
	GtkWidget *vbox_controls;

	/*
		--------------------------
		|[][][][palette[][][][][]|
		|------------------------|
		|       canvas     | con |
		|                  | trol|
		|                  | s   |
		|                  |     |
		|                  |     |
		--------------------------
	*/
	GtkWidget *clear_button;
	GtkWidget *save_button;

	GtkWidget *palette_color_box[PALETTE_NUM_COLORS];
	/* GdkPixbuf *palette_color_area[PALETTE_NUM_COLORS]; */

	GaimGtkWhiteboard *gtkwb = g_new0( GaimGtkWhiteboard, 1 );

	const char *window_title;

	gtkwb->wb = wb;
	wb->ui_data = gtkwb;

	/* Get dimensions (default?) for the whiteboard canvas */
	if( wb->prpl_ops && wb->prpl_ops->get_dimensions )
		wb->prpl_ops->get_dimensions( wb, &gtkwb->width, &gtkwb->height );

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtkwb->window = window;
	gtk_widget_set_name( window, wb->who );

	/* Try and set window title as the name of the buddy, else just use their
	 * username
	 */
	window_title = gaim_contact_get_alias( gaim_buddy_get_contact( gaim_find_buddy( wb->account, wb->who ) ) );
	if( window_title )
		gtk_window_set_title( ( GtkWindow* )( window ), window_title );
	else
		gtk_window_set_title( ( GtkWindow* )( window ), wb->who );

	gtk_window_set_resizable( ( GtkWindow* )( window ), FALSE );

	g_signal_connect( G_OBJECT( window ), "destroy",
			  G_CALLBACK( gaim_gtk_whiteboard_exit ), ( gpointer )( gtkwb ) );

	/* Create vertical box to place palette above the canvas and controls */
	vbox_palette_above_canvas_and_controls = gtk_vbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( window ), vbox_palette_above_canvas_and_controls );
	gtk_widget_show( vbox_palette_above_canvas_and_controls );

	/* Create horizontal box for the palette and all its entries */
	hbox_palette = gtk_hbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( vbox_palette_above_canvas_and_controls ), hbox_palette );
	gtk_widget_show( hbox_palette );

	/* Create horizontal box to seperate the canvas from the controls */
	hbox_canvas_and_controls = gtk_hbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( vbox_palette_above_canvas_and_controls ), hbox_canvas_and_controls );
	gtk_widget_show( hbox_canvas_and_controls );

	for( i = 0; i < PALETTE_NUM_COLORS; i++ )
	{
		/* palette_color_area[i] = */

		palette_color_box[i] = gtk_image_new_from_pixbuf( NULL );
		gtk_widget_set_size_request( palette_color_box[i], gtkwb->width / PALETTE_NUM_COLORS ,32 );
		gtk_container_add( GTK_CONTAINER( hbox_palette ), palette_color_box[i] );

		gtk_widget_show( palette_color_box[i] );
	}

	/* Create the drawing area */
	drawing_area = gtk_drawing_area_new();
	gtkwb->drawing_area = drawing_area;
	gtk_widget_set_size_request( GTK_WIDGET( drawing_area ), gtkwb->width, gtkwb->height );
	gtk_box_pack_start( GTK_BOX( hbox_canvas_and_controls ), drawing_area, TRUE, TRUE, 8 );

	gtk_widget_show( drawing_area );

	/* Signals used to handle backing pixmap */
	g_signal_connect( G_OBJECT( drawing_area ), "expose_event",
			  G_CALLBACK( gaim_gtk_whiteboard_expose_event ), ( gpointer )( gtkwb ) );

	g_signal_connect( G_OBJECT( drawing_area ), "configure_event",
			  G_CALLBACK( gaim_gtk_whiteboard_configure_event ), ( gpointer )( gtkwb ) );

	/* Event signals */
	g_signal_connect( G_OBJECT( drawing_area ), "button_press_event",
			  G_CALLBACK( gaim_gtk_whiteboard_brush_down ), ( gpointer )( gtkwb ) );

	g_signal_connect( G_OBJECT( drawing_area ), "motion_notify_event",
			  G_CALLBACK( gaim_gtk_whiteboard_brush_motion ), ( gpointer )( gtkwb ) );

	g_signal_connect( G_OBJECT( drawing_area ), "button_release_event",
			  G_CALLBACK( gaim_gtk_whiteboard_brush_up ), ( gpointer )( gtkwb ) );

	gtk_widget_set_events( drawing_area, GDK_EXPOSURE_MASK		|
					     GDK_LEAVE_NOTIFY_MASK	|
					     GDK_BUTTON_PRESS_MASK	|
					     GDK_POINTER_MOTION_MASK	|
					     GDK_BUTTON_RELEASE_MASK	|
					     GDK_POINTER_MOTION_HINT_MASK );

	/* Create vertical box to contain the controls */
	vbox_controls = gtk_vbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( hbox_canvas_and_controls ), vbox_controls );
	gtk_widget_show( vbox_controls );

	/* Add a clear button */
	clear_button = gtk_button_new_with_label( "Clear" );
	gtk_widget_set_size_request( clear_button, 96 ,32 );
	gtk_box_pack_start( GTK_BOX( vbox_controls ), clear_button, FALSE, FALSE, 0 );
	gtk_widget_show( clear_button );

	g_signal_connect( G_OBJECT( clear_button ), "clicked",
			  G_CALLBACK( gaim_gtk_whiteboard_button_clear_press ), ( gpointer )( gtkwb ) );

	/* Add a save button */
	save_button = gtk_button_new_with_label( "Save" );
	gtk_widget_set_size_request( save_button, 96 ,32 );
	gtk_box_pack_start( GTK_BOX( vbox_controls ), save_button, FALSE, FALSE, 8 );
	gtk_widget_show( save_button );

	g_signal_connect( G_OBJECT( save_button ), "clicked",
			  G_CALLBACK( gaim_gtk_whiteboard_button_save_press ), ( gpointer )( gtkwb ) );

	/* Make all this (window) visible */
	gtk_widget_show( window );

	gaim_gtk_whiteboard_set_canvas_as_icon( gtkwb );

	/* TODO Specific protocol/whiteboard assignment here? Needs a UI Op? */
	/* Set default brush size and color */
	/*
	ds->brush_size	= DOODLE_BRUSH_MEDIUM;
	ds->brush_color	= 0;
	*/
}

void gaim_gtk_whiteboard_destroy( GaimWhiteboard *wb )
{
	GaimGtkWhiteboard *gtkwb = wb->ui_data;

	/* TODO Ask if user wants to save picture before the session is closed */

	/* Clear graphical memory */
	if( gtkwb->pixmap )
	{
		g_object_unref( gtkwb->pixmap );
		gtkwb->pixmap = NULL;
	}

	if( gtkwb->window )
	{
		gtk_widget_destroy( gtkwb->window );
		gtkwb->window = NULL;
	}
}

void gaim_gtk_whiteboard_exit( GtkWidget *widget, gpointer data )
{
	GaimGtkWhiteboard	*gtkwb	= ( GaimGtkWhiteboard* )( data );
	GaimWhiteboard		*wb	= gtkwb->wb;

	if( gtkwb->window && gtkwb->pixmap )
	{
		LocalShutdownRequest = TRUE;

		gaim_gtk_whiteboard_destroy( wb );
	}
	else
		LocalShutdownRequest = FALSE;

	if( gtkwb )
	{
		g_free( gtkwb );

		gtkwb = NULL;
		wb->ui_data = NULL;
	}

	/* Destroy whiteboard core, if the local user exited the whiteboard window */
	if( wb && LocalShutdownRequest )
	{
		gaim_whiteboard_destroy( wb );
		wb = NULL;
	}
}

/*
 * Whiteboard start button on conversation window (move this code to gtkconv?
 * and use new prpl_info member?)
 */
#if 0
void gaim_gtkwhiteboard_button_start_press( GtkButton *button, gpointer data )
{
	GaimConversation	*conv		= data;
	GaimAccount		*account	= gaim_conversation_get_account( conv );
	GaimConnection		*gc		= gaim_account_get_connection( account );
	char			*to		= ( char* )( gaim_conversation_get_name( conv ) );

	/* Only handle this if local client requested Doodle session (else local
	 * client would have sent one)
	 */
	GaimWhiteboard	*wb			= gaim_whiteboard_get( account, to );

	/* Write a local message to this conversation showing that a request for a
	 * Doodle session has been made
	 */
	gaim_conv_im_write( GAIM_CONV_IM( conv ), "", _("Sent Doodle request."),
			    GAIM_MESSAGE_NICK | GAIM_MESSAGE_RECV, time( NULL ) );

	yahoo_doodle_command_send_request( gc, to );
	yahoo_doodle_command_send_ready( gc, to );

	/* Insert this 'session' in the list.  At this point, it's only a requested
	 * session.
	 */
	wb = gaim_whiteboard_create( account, to, DOODLE_STATE_REQUESTING );
}
#endif

gboolean gaim_gtk_whiteboard_configure_event( GtkWidget *widget, GdkEventConfigure *event, gpointer data )
{
	GaimGtkWhiteboard	*gtkwb	= ( GaimGtkWhiteboard* )( data );

	GdkPixmap		*pixmap = gtkwb->pixmap;

	if( pixmap )
		g_object_unref( pixmap );

	pixmap = gdk_pixmap_new( widget->window,
				 widget->allocation.width,
				 widget->allocation.height,
				 -1 );

	gtkwb->pixmap = pixmap;

	gdk_draw_rectangle( pixmap,
			    widget->style->white_gc,
			    TRUE,
			    0, 0,
			    widget->allocation.width,
			    widget->allocation.height );

	return( TRUE );
}

gboolean gaim_gtk_whiteboard_expose_event( GtkWidget *widget, GdkEventExpose *event, gpointer data )
{
	GaimGtkWhiteboard	*gtkwb	= ( GaimGtkWhiteboard* )( data );
	GdkPixmap		*pixmap	= gtkwb->pixmap;

	gdk_draw_drawable( widget->window,
			   widget->style->fg_gc[GTK_WIDGET_STATE( widget )],
			   pixmap,
			   event->area.x, event->area.y,
			   event->area.x, event->area.y,
			   event->area.width, event->area.height );

	return( FALSE );
}

gboolean gaim_gtk_whiteboard_brush_down( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
	GaimGtkWhiteboard	*gtkwb		= ( GaimGtkWhiteboard* )( data );
	GdkPixmap		*pixmap		= gtkwb->pixmap;

	GaimWhiteboard		*wb		= gtkwb->wb;
	GList			*draw_list	= wb->draw_list;

	int			*x0		= NULL;
	int			*y0		= NULL;

	if( BrushState != BRUSH_STATE_UP )
	{
		/* Potential double-click DOWN to DOWN? */
		BrushState = BRUSH_STATE_DOWN;

		/* return( FALSE ); */
	}

	BrushState = BRUSH_STATE_DOWN;

	if( event->button == 1 && pixmap != NULL )
	{
		/* Check if draw_list has contents; if so, clear it */
		if( draw_list )
			draw_list = gaim_whiteboard_draw_list_destroy( draw_list );

		x0		= g_new0( int, 1 );
		y0		= g_new0( int, 1 );

		*x0		= event->x;
		*y0		= event->y;

		/* Set tracking variables */
		LastX		= *x0;
		LastY		= *y0;

		MotionCount	= 0;

		draw_list	= g_list_append( draw_list, ( gpointer )( x0 ) );
		draw_list	= g_list_append( draw_list, ( gpointer )( y0 ) );

		gaim_gtk_whiteboard_draw_brush_point( gtkwb->wb,
						      event->x, event->y,
						      0,5 );
		/* gtkwb->brush_color, gtkwb->brush_size ); NOTE temp const prot uiop */
	}

	wb->draw_list = draw_list;

	return( TRUE );
}

gboolean gaim_gtk_whiteboard_brush_motion( GtkWidget *widget, GdkEventMotion *event, gpointer data )
{
	int			x;
	int			y;
	int			*dx;
	int			*dy;

	GdkModifierType		state;

	GaimGtkWhiteboard	*gtkwb		= ( GaimGtkWhiteboard* )( data );
	GdkPixmap		*pixmap		= gtkwb->pixmap;

	GaimWhiteboard		*wb		= gtkwb->wb;
	GList			*draw_list	= wb->draw_list;

	if( event->is_hint )
		gdk_window_get_pointer( event->window, &x, &y, &state );
	else
	{
		x	= event->x;
		y	= event->y;
		state	= event->state;
	}

	if( state & GDK_BUTTON1_MASK && pixmap != NULL )
	{
		if( ( BrushState != BRUSH_STATE_DOWN ) && ( BrushState != BRUSH_STATE_MOTION ) )
		{
			g_print( "***Bad brush state transition %d to MOTION\n", BrushState );

			BrushState = BRUSH_STATE_MOTION;

			return( FALSE );
		}
		BrushState = BRUSH_STATE_MOTION;

		dx	= g_new0( int, 1 );
		dy	= g_new0( int, 1 );

		*dx	= x - LastX;
		*dy	= y - LastY;

		MotionCount++;

		/* NOTE 100 is a temporary constant for how many deltas/motions in a
		 * stroke (needs UI Ops?)
		 */
		if( MotionCount == 100 )
		{
			int *x0			= g_new0( int, 1 );
			int *y0			= g_new0( int, 1 );

			draw_list = g_list_append( draw_list, ( gpointer )( dx ) );
			draw_list = g_list_append( draw_list, ( gpointer )( dy ) );

			/* Send draw list to prpl draw_list handler */
			if( gtkwb->wb->prpl_ops && gtkwb->wb->prpl_ops->send_draw_list )
				gtkwb->wb->prpl_ops->send_draw_list( gtkwb->wb, draw_list );

			/* The brush stroke is finished, clear the list for another one */
			if( draw_list )
				draw_list = gaim_whiteboard_draw_list_destroy( draw_list );

			*x0			= LastX;
			*y0			= LastY;

			/* Reset motion tracking */
			MotionCount		= 0;

			draw_list		= g_list_append( draw_list, ( gpointer )( x0 ) );
			draw_list		= g_list_append( draw_list, ( gpointer )( y0 ) );

			dx	= g_new0( int, 1 );
			dy	= g_new0( int, 1 );

			*dx	= x - LastX;
			*dy	= y - LastY;
		}

		draw_list = g_list_append( draw_list, ( gpointer )( dx ) );
		draw_list = g_list_append( draw_list, ( gpointer )( dy ) );

		gaim_gtk_whiteboard_draw_brush_line( gtkwb->wb,
						     LastX, LastY,
						     x, y,
						     0, 5 );
		/*gtkwb->brush_color, gtkwb->brush_size ); temp const proto ui ops? */

		/* Set tracking variables */
		LastX = x;
		LastY = y;
	}

	wb->draw_list = draw_list;

	return( TRUE );
}

gboolean gaim_gtk_whiteboard_brush_up( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
	GaimGtkWhiteboard	*gtkwb		= ( GaimGtkWhiteboard* )( data );
	GdkPixmap		*pixmap		= gtkwb->pixmap;

	GaimWhiteboard		*wb		= gtkwb->wb;
	GList			*draw_list	= wb->draw_list;

	if( ( BrushState != BRUSH_STATE_DOWN ) && ( BrushState != BRUSH_STATE_MOTION ) )
	{
		g_print( "***Bad brush state transition %d to UP\n", BrushState );

		BrushState = BRUSH_STATE_UP;

		return( FALSE );
	}
	BrushState = BRUSH_STATE_UP;

	if( event->button == 1 && pixmap != NULL )
	{
		/* If the brush was never moved, express two sets of two deltas That's a
		 * 'point,' but not for Yahoo!
		 */
		/* if( ( event->x == LastX ) && ( event->y == LastY ) ) */
		if( MotionCount == 0 )
		{
			int index;

			for( index = 0; index < 2; index++ )/* NOTE Yahoo Doodle specific! */
			{
				int *x0		= NULL;
				int *y0		= NULL;

				x0		= g_new0( int, 1 );
				y0		= g_new0( int, 1 );

				draw_list	= g_list_append( draw_list, ( gpointer )( x0 ) );
				draw_list	= g_list_append( draw_list, ( gpointer )( y0 ) );
			}
		}
		/*
		else
			MotionCount = 0;
		*/

		/* Send draw list to prpl draw_list handler */
		if( gtkwb->wb->prpl_ops && gtkwb->wb->prpl_ops->send_draw_list )
			gtkwb->wb->prpl_ops->send_draw_list( gtkwb->wb, draw_list );

		gaim_gtk_whiteboard_set_canvas_as_icon( gtkwb );

		/* The brush stroke is finished, clear the list for another one */
		if( draw_list )
			draw_list = gaim_whiteboard_draw_list_destroy( draw_list );

		wb->draw_list = draw_list;
	}

	return( TRUE );
}

/* void gaim_gtk_whiteboard_draw_brush_point( GtkWidget *widget, GaimGtkWhiteboard *gtkwb,
					  int x, int y, int color, int size )
*/
void gaim_gtk_whiteboard_draw_brush_point( GaimWhiteboard *wb, int x, int y, int color, int size )
{
	GaimGtkWhiteboard	*gtkwb	= wb->ui_data;
	GtkWidget		*widget	= gtkwb->drawing_area;
	GdkPixmap 		*pixmap	= gtkwb->pixmap;

	GdkRectangle		update_rect;

	GdkGC			*gfx_con = gdk_gc_new( pixmap );
	GdkColor		col;

	update_rect.x		= x - size / 2;
	update_rect.y		= y - size / 2;
	update_rect.width	= size;
	update_rect.height	= size;

	/* Interpret and convert color */
	gaim_gtk_whiteboard_rgb24_to_rgb48( color, &col );

	gdk_gc_set_rgb_fg_color( gfx_con, &col );
	/* gdk_gc_set_rgb_bg_color( gfx_con, &col ); */

	/* NOTE 5 is a size constant for now... this is because of how poorly the
	 * gdk_draw_arc draws small circles
	 */
	if( size < 5 )
	{
		/* Draw a rectangle/square */
		gdk_draw_rectangle( pixmap,
				    gfx_con,
				    TRUE,
				    update_rect.x, update_rect.y,
				    update_rect.width, update_rect.height );
	}
	else
	{
		/* Draw a circle */
		gdk_draw_arc( pixmap,
			      gfx_con,
			      TRUE,
			      update_rect.x, update_rect.y,
			      update_rect.width, update_rect.height,
			      0, FULL_CIRCLE_DEGREES );
	}

	gtk_widget_queue_draw_area( widget, 
				    update_rect.x, update_rect.y,
				    update_rect.width, update_rect.height );

	gdk_gc_unref( gfx_con );
}

/* Uses Bresenham's algorithm (as provided by Wikipedia) */
/* void gaim_gtk_whiteboard_draw_brush_line( GtkWidget *widget, GaimGtkWhiteboard *gtkwb,
 					 int x0, int y0, int x1, int y1, int color, int size )
*/
void gaim_gtk_whiteboard_draw_brush_line( GaimWhiteboard *wb, int x0, int y0, int x1, int y1, int color, int size )
{
	int temp;

	int xstep;
	int ystep;

	int dx;
	int dy;

	int error;
	int derror;

	int x;
	int y;

	gboolean steep = abs( y1 - y0 ) > abs( x1 - x0 );

	if( steep )
	{
		temp = x0; x0 = y0; y0 = temp;
		temp = x1; x1 = y1; y1 = temp;
	}

	dx		= abs( x1 - x0 );
	dy		= abs( y1 - y0 );

	error	= 0;
	derror	= dy;

	x		= x0;
	y		= y0;

	if( x0 < x1 )
		xstep = 1;
	else
		xstep = -1;

	if( y0 < y1 )
		ystep = 1;
	else
		ystep = -1;

	if( steep )
		gaim_gtk_whiteboard_draw_brush_point( wb, y, x, color, size );
	else
		gaim_gtk_whiteboard_draw_brush_point( wb, x, y, color, size );

	while( x != x1 )
	{
		x	= x + xstep;
		error	= error + derror;

		if( ( error * 2 ) >= dx )
		{
			y	= y + ystep;
			error	= error - dx;
		}

		if( steep )
			gaim_gtk_whiteboard_draw_brush_point( wb, y, x, color, size );
		else
			gaim_gtk_whiteboard_draw_brush_point( wb, x, y, color, size );
	}
}

void gaim_gtk_whiteboard_set_dimensions( GaimWhiteboard *wb, int width, int height )
{
	GaimGtkWhiteboard *gtkwb = wb->ui_data;

	gtkwb->width	= width;
	gtkwb->height	= height;
}

void gaim_gtk_whiteboard_clear( GaimWhiteboard *wb )
{
	GaimGtkWhiteboard	*gtkwb		= wb->ui_data;
	GdkPixmap		*pixmap		= gtkwb->pixmap;
	GtkWidget		*drawing_area	= gtkwb->drawing_area;

	gdk_draw_rectangle( pixmap,
			    drawing_area->style->white_gc,
			    TRUE,
			    0, 0,
			    drawing_area->allocation.width, drawing_area->allocation.height );

	gtk_widget_queue_draw_area( drawing_area,
				    0, 0,
				    drawing_area->allocation.width, drawing_area->allocation.height );
}

void gaim_gtk_whiteboard_button_clear_press( GtkWidget *widget, gpointer data )
{
	GaimGtkWhiteboard	*gtkwb	= ( GaimGtkWhiteboard* )( data );

	gaim_gtk_whiteboard_clear( gtkwb->wb );

	gaim_gtk_whiteboard_set_canvas_as_icon( gtkwb );

	/* Do protocol specific clearing procedures */
	if( gtkwb->wb->prpl_ops && gtkwb->wb->prpl_ops->clear )
		gtkwb->wb->prpl_ops->clear( gtkwb->wb );
}

void gaim_gtk_whiteboard_button_save_press( GtkWidget *widget, gpointer data )
{
	GaimGtkWhiteboard	*gtkwb	= ( GaimGtkWhiteboard* )( data );
	GdkPixbuf		*pixbuf;

	GtkWidget *dialog;

	int result;

#if GTK_CHECK_VERSION(2,4,0) /* FILECHOOSER */
	dialog = gtk_file_chooser_dialog_new ("Save File",
					      GTK_WINDOW(gtkwb->window),
					      GTK_FILE_CHOOSER_ACTION_SAVE,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					      NULL );

	/* gtk_file_chooser_set_do_overwrite_confirmation( GTK_FILE_CHOOSER( dialog ), (gboolean)(TRUE) ); */

/*	if( user_edited_a_new_document ) */
	{
/*		gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( dialog ), default_folder_for_saving ); */
		gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER( dialog ), "whiteboard.jpg" );
	}
/*
	else
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), filename_for_existing_document);
*/
#else
	dialog = gtk_file_selection_new("Save File");
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(dialog), "whiteboard.jpg");
#endif
	result = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( result == GTK_RESPONSE_ACCEPT )
	{
		char *filename;

#if GTK_CHECK_VERSION(2,4,0) /* FILECHOOSER */
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER( dialog ) );
#else
		filename = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog)));
#endif
		gtk_widget_destroy( dialog );

		/* Makes an icon from the whiteboard's canvas 'image' */
		pixbuf = gdk_pixbuf_get_from_drawable( NULL,
						( GdkDrawable* )( gtkwb->pixmap ),
						gdk_drawable_get_colormap( gtkwb->pixmap ),
						0, 0,
						0, 0,
						gtkwb->width, gtkwb->height );

		if( gdk_pixbuf_save( pixbuf,
		    		     filename,
				     "jpeg",
				     NULL,
				     "quality",
				     "100",
				     NULL ) )
			g_print( "File Saved...\n" );
		else
			g_print( "File not Saved... Error\n" );
		g_free(filename);
	}
	else
	if( result == GTK_RESPONSE_CANCEL )
	{
		gtk_widget_destroy( dialog );

		g_print( "File not Saved... Canceled\n" );
	}
}

void gaim_gtk_whiteboard_set_canvas_as_icon( GaimGtkWhiteboard *gtkwb )
{
	GdkPixbuf *pixbuf;

	/* Makes an icon from the whiteboard's canvas 'image' */
	pixbuf = gdk_pixbuf_get_from_drawable( NULL,
					       ( GdkDrawable* )( gtkwb->pixmap ),
					       gdk_drawable_get_colormap( gtkwb->pixmap ),
					       0, 0,
					       0, 0,
					       gtkwb->width, gtkwb->height );

	gtk_window_set_icon( ( GtkWindow* )( gtkwb->window ), pixbuf );
}

void gaim_gtk_whiteboard_rgb24_to_rgb48( int color_rgb, GdkColor *color )
{
	color->red	= ( color_rgb >> 8 ) | 0xFF;
	color->green	= ( color_rgb & 0xFF00 ) | 0xFF;
	color->blue	= ( ( color_rgb & 0xFF ) << 8 ) | 0xFF;
}

