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
#include "lookuptable.h"

#define YFORSVG  63

void svg_set_size(gint *width, gint *height, gpointer user_data)
{
//  g_print("w = %d h = %d\n", *width, *height);
  *width = *height = GPOINTER_TO_SIZE(user_data);
}

static void
draw_inputed(GdkDrawable * gw, GdkGC * gc, GString * inputed,GdkPixbuf * icons[])
{
  int i;
  GdkPixbuf * px;
  GdkPixbuf * icon;

  for (i = 0; i < inputed ->len; i++)
	{
		switch (inputed->str[i])
		{
		case 'h':
			icon = icons[0];
			break;
		case 's':
			icon = icons[1];
			break;
		case 'p':
			icon = icons[2];
			break;
		case 'n':
			icon = icons[3];
			break;
		case 'z':
			icon = icons[4];
			break;
		}
      px = gdk_pixbuf_scale_simple(icon,14,14,GDK_INTERP_HYPER);
      gdk_draw_pixbuf(gw,gc,px,0,0,i*15+5,4,14,14,GDK_RGB_DITHER_NONE,0,0);
      g_object_unref(px);
    }
}

gboolean
on_paint(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{

  GdkGC * gc;
  GdkPixbuf * pixbuf;
  IBusT9Engine * engine;
  GdkWindow* gw;

  PangoLayout * ly;

  engine = (IBusT9Engine *) (user_data);

  int size = 37;

  gw = widget->window;

  gc = gdk_gc_new(gw);

  int i;

  gdk_gc_set_clip_region (gc,event->region);

  //画已经输入的笔画
  if (widget, engine->inputed->len)
    {
      draw_inputed(gw,gc,engine->inputed,engine->keysicon);
    }

  g_object_unref(gc);
  return FALSE;
}

gboolean
on_mouse_move(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
  int i;
  IBusT9Engine * engine;
  GdkWindow * gw;
  GdkRectangle regtangle;
  GdkRegion * reg;

  engine = (IBusT9Engine *) (user_data);

  gw = widget->window;

  if (engine->drag)
    {
      engine->laststate.x = event->x_root - engine->lastpoint.x;
      engine->laststate.y = event->y_root - engine->lastpoint.y;
      gtk_window_move(GTK_WINDOW(widget), engine->laststate.x,
          engine->laststate.y);
    }
  return TRUE;
}

void button_clicked(GtkButton *button, gpointer user_data)
{
	  int i;
	  IBusT9Engine * engine;
	  IBusT9EngineClass *ibus;
	  GdkRegion * reg;
	  GdkRectangle regtangle;

	  engine = ((struct button_data *)user_data)->engine;
	  ibus = IBUS_T9_ENGINE_GET_CLASS(engine);

	  i = ((struct button_data *)user_data)->index;

      static char bihua[5][8] =
        { "横", "竖", "撇", "捺", "折" };

      g_printf("%s clicked\n", bihua[i]);

      ibus->parent.process_key_event(IBUS_ENGINE(engine), IBUS_KP_1 + i, IBUS_KP_1 + i, 0);

      gdk_window_invalidate_rect(engine->LookupTable->window, 0, 0);
}

void table_button_clicked(GtkButton *button, gpointer user_data)
{
	  int i;
	  IBusT9Engine * engine;
	  IBusT9EngineClass *ibus;
	  GdkRegion * reg;
	  GdkRectangle regtangle;

	  engine = ((struct button_data *)user_data)->engine;
	  ibus = IBUS_T9_ENGINE_GET_CLASS(engine);

	  i = ((struct button_data *)user_data)->index;

	  ibus->commit_string(engine,i);
}


gboolean
on_button(GtkWidget* widget, GdkEventButton *event, gpointer user_data)
{
  int i;
  IBusT9Engine * engine;
  IBusT9EngineClass *klass;
  GdkRegion * reg;
  GdkRectangle regtangle;

  engine = (IBusT9Engine *) (user_data);
  klass = IBUS_T9_ENGINE_GET_CLASS(engine);

  engine->drag = event->button != 1;

  if (event->button != 1)
	{
		engine->lastpoint.x = event->x;
		engine->lastpoint.y = event->y;
		return FALSE;
	}

  return TRUE;
}

void widget_realize(GtkWidget *widget, gpointer user_data)
{
	  GdkRegion * region;
	  GdkPixmap * pxmp;
	  GdkGC * gc;
	  GdkWindow * gw;
	  GdkColor black, white;

	  IBusT9Engine * engine;

	  engine = (IBusT9Engine *) (user_data);

	  GdkColormap* colormap = gdk_colormap_get_system();

	  gw = widget->window;

	  gdk_window_get_size(gw,&engine->laststate.width,&engine->laststate.height);

	  g_print("width is %d \n",engine->laststate.width);

	  gdk_color_black(colormap, &black);
	  gdk_color_white(colormap, &white);

	  g_object_unref(colormap);

	  //  region = gdk_region_new();
	  pxmp = gdk_pixmap_new(NULL, engine->laststate.width, engine->laststate.height, 1);
	  gc = gdk_gc_new(GDK_DRAWABLE(pxmp));

	  gdk_gc_set_foreground(gc, &black);

	  gdk_draw_rectangle(GDK_DRAWABLE(pxmp),gc,1,0,0,engine->laststate.width,engine->laststate.height);

	  gdk_gc_set_foreground(gc, &white);

	  gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1, 0, 0, 30, 30, 0, 360 * 64);
	  gdk_draw_arc(GDK_DRAWABLE(pxmp), gc, 1, engine->laststate.width - 30,0,30,30,0,360 * 64);
	  gdk_draw_arc(GDK_DRAWABLE(pxmp),gc,1,engine->laststate.width - 30,engine->laststate.height - 30,30,30,0,360 * 64);
	  gdk_draw_arc(GDK_DRAWABLE(pxmp),gc,1,0,engine->laststate.height - 30,30,30,0,360 * 64);
	  gdk_draw_rectangle(GDK_DRAWABLE(pxmp),gc,1,0,15,engine->laststate.width ,engine->laststate.height - 30);
	  gdk_draw_rectangle(GDK_DRAWABLE(pxmp),gc,1,15,0,engine->laststate.width - 30,engine->laststate.height);

	  gdk_window_shape_combine_mask(gw,pxmp,0,0);

	  g_object_unref(gc);
	  g_object_unref(pxmp);

}
