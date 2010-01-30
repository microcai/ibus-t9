/*
 * lookuptable.c
 *
 *  Created on: 2010-1-30
 *      Author: cai
 */
#include <config.h>

#ifdef HAVE_GETTEXT

#include <locale.h>
#include <libintl.h>

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#else

#define _(x) (x)

#endif

#include <string.h>
#include <cairo/cairo.h>
#include "engine.h"
#include <librsvg/rsvg-cairo.h>
#include "lookuptable.h"

static void svg_set_size(gint *width, gint *height, gpointer user_data)
{
	g_print("w = %d h = %d\n", *width, *height);
	*width = *height = GPOINTER_TO_SIZE(user_data);
}

gboolean on_paint(GtkWidget *widget, GdkEventExpose *event,
		gpointer user_data)
{
	int const YFORSVG =  50;
	GdkGC	* gc;
	GdkPixbuf * pixbuf;
	IBusT9Engine * engine;

	engine = (IBusT9Engine *) (user_data);

	int size = 37;

	gc = gdk_gc_new(widget->window);

	int i;

	for(i=0;i<5;i++) //画笔画
	{
		rsvg_handle_set_size_callback(engine->keysicon[i],svg_set_size,GSIZE_TO_POINTER(37),0);

		pixbuf = rsvg_handle_get_pixbuf(engine->keysicon[i]);

		gdk_draw_pixbuf(widget->window,gc,pixbuf,0,0,i*40 + 1,  YFORSVG,37,37,GDK_RGB_DITHER_NONE,0,0);

		g_object_unref(pixbuf);
	}
	//画已经输入的笔画


	//画候选字



	g_object_unref(gc);
	return TRUE;

}

gboolean on_mouse_move(GtkWidget *widget, GdkEventMotion *event,	gpointer user_data)
{
	IBusT9Engine * engine;

	engine = (IBusT9Engine *) (user_data);

	if (engine->drag)
	{
		engine->laststate.x = event->x_root - engine->lastpoint.x;
		engine->laststate.y = event->y_root - engine->lastpoint.y;
		gtk_window_move(GTK_WINDOW(widget), engine->laststate.x,
				engine->laststate.y);
	}
	return FALSE;
}

gboolean on_button(GtkWidget* widget, GdkEventButton *event, gpointer user_data)
{
	IBusT9Engine * engine;
	LineStoken * token;

	engine = (IBusT9Engine *) (user_data);

	switch (event->type)
	{

	case GDK_BUTTON_PRESS:
		if(event->button != 1)
		{
			engine->drag = TRUE;
			engine->lastpoint.x = event->x;
			engine->lastpoint.y = event->y;
			return ;
		}else
		{
			engine->drag = FALSE;
		}
		break;
	case GDK_BUTTON_RELEASE:break;
	}
}
