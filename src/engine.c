/* vim:set et sts=4: */

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

#include "engine.h"
#include "lookuptable.h"
#include "../icons/ibus_t9.icons.h"

/* functions prototype */
static void
ibus_t9_engine_class_init(IBusT9EngineClass *klass);
static void
ibus_t9_engine_init(IBusT9Engine *engine);
static void
ibus_t9_engine_destroy(IBusT9Engine *engine);
static gboolean
ibus_t9_engine_process_key_event(IBusEngine *engine, guint keyval,
    guint keycode, guint modifiers);
static void
ibus_t9_engine_focus_in(IBusEngine *engine);
static void
ibus_t9_engine_focus_out(IBusEngine *engine);
static void
ibus_t9_engine_reset(IBusEngine *engine);
static void
ibus_engine_set_cursor_location(IBusEngine *engine, gint x, gint y, gint w,
    gint h);
static void
ibus_t9_engine_set_capabilities(IBusEngine *engine, guint caps);
static void
ibus_t9_engine_page_up(IBusEngine *engine);
static void
ibus_t9_engine_page_down(IBusEngine *engine);
static void
ibus_t9_engine_cursor_up(IBusEngine *engine);
static void
ibus_t9_engine_cursor_down(IBusEngine *engine);
static void
ibus_enchant_property_activate(IBusEngine *engine, const gchar *prop_name,
    gint prop_state);
static void
ibus_t9_engine_property_show(IBusEngine *engine, const gchar *prop_name);
static void
ibus_t9_engine_property_hide(IBusEngine *engine, const gchar *prop_name);
static int
ibus_t9_engine_commit_string(IBusT9Engine *engine, guint index);

GType
ibus_t9_engine_get_type(void)
{
  static GType type = 0;

  static const GTypeInfo type_info =
    { sizeof(IBusT9EngineClass), (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL, (GClassInitFunc) ibus_t9_engine_class_init,
        NULL, NULL, sizeof(IBusT9Engine), 0,
        (GInstanceInitFunc) ibus_t9_engine_init, };

  if (type == 0)
    {
      type = g_type_register_static(IBUS_TYPE_ENGINE, "IBusT9Engine",
          &type_info, (GTypeFlags) 0);
    }

  return type;
}

static void
ibus_t9_engine_class_init(IBusT9EngineClass *klass)
{
  IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
  IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

  ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_t9_engine_destroy;

  engine_class->process_key_event = ibus_t9_engine_process_key_event;

  engine_class->enable = ibus_t9_engine_focus_in;

  engine_class->disable = ibus_t9_engine_focus_out;

  engine_class->focus_in = ibus_t9_engine_focus_in;

  engine_class->focus_out = ibus_t9_engine_focus_out;

  klass->icondir = g_string_new(icondir);

  if((klass->phraser = phraser_new(DATAFILE)))
      phraser_optimise(klass->phraser);
  else g_error(_("cannot open %s"),DATAFILE);

  klass->commit_string = ibus_t9_engine_commit_string;
}

static void
ibus_t9_engine_init(IBusT9Engine *engine)
{
  size_t i;
  IBusT9EngineClass* klass;
  GdkPixbuf * px;

  GError * err;

  engine->laststate.x = 300;
  engine->laststate.y = 300;
  engine->laststate.width = 200;
  engine->laststate.height = 100;
  engine->drag = 0;

  klass = IBUS_T9_ENGINE_GET_CLASS(engine);

  engine->LookupTable = gtk_window_new(GTK_WINDOW_POPUP);

  engine->box = gtk_vbox_new(FALSE,0);

  gtk_container_add(GTK_CONTAINER(engine->LookupTable),engine->box);

  GtkWidget * hb = gtk_hbox_new(TRUE,0);

  gtk_box_pack_end_defaults(GTK_BOX(engine->box),hb);

  const guint8 * ibus_t9_icon_key[] =
  {	  ibus_t9_icon_key1, ibus_t9_icon_key2, ibus_t9_icon_key3, ibus_t9_icon_key4, ibus_t9_icon_key5 };

  for (i = 0; i < 5; ++i)
    {
      err = NULL;
      gchar *  icon_file ;

      struct button_data * callback_data = & engine->stok_botton_call_back[i];
      callback_data->engine = engine;
      callback_data->index = i;

      engine->keysicon[i] = gdk_pixbuf_new_from_inline(-1,ibus_t9_icon_key[i],FALSE,NULL);

      px = gdk_pixbuf_scale_simple(engine->keysicon[i],32,32,GDK_INTERP_HYPER);
      GtkWidget * gtkimg = gtk_image_new_from_pixbuf(px);
      g_object_unref(px);

      GtkWidget* bt = gtk_button_new();
      gtk_button_set_image(GTK_BUTTON(bt),gtkimg);
	  gtk_box_pack_start_defaults(GTK_BOX(hb),bt);
	  g_signal_connect(G_OBJECT(bt),"clicked",G_CALLBACK(button_clicked),callback_data);
    }

  GtkWidget * head = gtk_image_new();

  gtk_widget_set_size_request(head,200,30);

  gtk_box_pack_start(GTK_BOX(engine->box),head,TRUE,TRUE,FALSE);

  g_signal_connect(G_OBJECT(head),"expose-event",G_CALLBACK(on_paint),engine);

  engine->tables = gtk_table_new(2, 5, TRUE);

  gtk_widget_set_size_request(engine->tables,200,80);

  gtk_box_pack_start(GTK_BOX(engine->box),engine->tables,TRUE,TRUE,TRUE);

  gtk_window_move(GTK_WINDOW(engine->LookupTable), engine->laststate.x,
      engine->laststate.y);

//gtk_window_resize(GTK_WINDOW(engine->LookupTable),200,200);

  gtk_widget_add_events(GTK_WIDGET(engine->LookupTable), GDK_BUTTON_MOTION_MASK
      | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  g_signal_connect_after(G_OBJECT(engine->LookupTable),"motion_notify_event",G_CALLBACK(on_mouse_move),engine);
  g_signal_connect(G_OBJECT(engine->LookupTable),"button-release-event",G_CALLBACK(on_button),engine);
  g_signal_connect(G_OBJECT(engine->LookupTable),"button-press-event",G_CALLBACK(on_button),engine);
  g_signal_connect(G_OBJECT(engine->LookupTable),"realize",G_CALLBACK(widget_realize),engine);

  engine->matched = g_array_sized_new(FALSE, TRUE, sizeof(MATCHED), 20);
  engine->inputed = g_string_new("");
  gtk_widget_show_all(engine->LookupTable);
}

static void
ibus_t9_engine_destroy(IBusT9Engine *engine)
{
  g_print("%s\n", __func__);//, engine->laststate.x, engine->laststate.y);
  if (engine->LookupTable)
    {
      gtk_widget_destroy(engine->LookupTable);
      engine->LookupTable = NULL;
      g_array_free(engine->matched, TRUE);
      g_string_free(engine->inputed, TRUE);
    }
  IBUS_OBJECT_CLASS(g_type_class_peek_parent(IBUS_ENGINE_GET_CLASS(engine)))->destroy(IBUS_OBJECT(engine));
}

static int ibus_t9_engine_commit_string(IBusT9Engine *engine, guint index)
{
	if (engine->matched->len > index)
	{
		ibus_engine_commit_text((IBusEngine *) engine,
				ibus_text_new_from_static_string(
						g_array_index(engine->matched,MATCHED,index).hanzi));
		g_string_truncate(engine->inputed, 0);
		return TRUE;
	}
	return FALSE;
}

static gboolean
ibus_t9_engine_update(IBusT9Engine *engine)
{
//	gtk_container_remove(GTK_CONTAINER(engine->box),engine->tables);

	gtk_widget_destroy(engine->tables);

	engine->tables = gtk_table_new(2, 5, TRUE);

	gtk_widget_set_size_request(engine->tables, 200, 80);

	gtk_box_pack_start(GTK_BOX(engine->box), engine->tables, TRUE, TRUE, TRUE);

  if (engine->inputed->len)
    {
      g_print("input is now %s\n", engine->inputed->str);
      phraser_get_phrases(engine->matched, engine->inputed,
          IBUS_T9_ENGINE_GET_CLASS(engine)->phraser);
    }
  else
	  engine->matched = g_array_set_size(engine->matched,0);
  int i;

  for( i =0; i < MIN(engine->matched->len,10) ; ++i )
  {
	  struct button_data * callback_data = & engine->table_botton_call_back[i];

	  callback_data->engine = engine;

	  callback_data->index = i;

	  g_print("create button with %s\n",g_array_index(engine->matched,MATCHED,i).hanzi);

	  GtkWidget * child = gtk_button_new_with_label(g_array_index(engine->matched,MATCHED,i).hanzi);

	  gtk_table_attach_defaults(GTK_TABLE(engine->tables), child, i % 5, i % 5 + 1,	i / 5, i / 5 + 1);

	  g_signal_connect(G_OBJECT(child),"clicked",G_CALLBACK(table_button_clicked),callback_data);

	  gtk_widget_show(child);
  }

  gtk_widget_show(engine->tables);

 //gdk_window_invalidate_rect(engine->LookupTable->window,0,0);
 return TRUE;
}

#define is_alpha(c) (((c) >= IBUS_a && (c) <= IBUS_z) || ((c) >= IBUS_A && (c) <= IBUS_Z))

static gboolean
ibus_t9_engine_process_key_event(IBusEngine *ibusengine, guint keyval,
    guint keycode, guint modifiers)
{


  IBusText *text;
  IBusT9Engine *engine = (IBusT9Engine *) ibusengine;

  if (modifiers & IBUS_RELEASE_MASK)
    return FALSE;

  modifiers &= (IBUS_CONTROL_MASK | IBUS_MOD1_MASK);

  g_printf("%s\n",__func__);

  switch (keyval)
    {
  case IBUS_space:
  case IBUS_Return:
	  if(engine->inputed->len)
		  return ibus_t9_engine_commit_string(engine, 0);
	  else return FALSE;
  case IBUS_Escape:

    return TRUE;

  case IBUS_Left:
    return TRUE;

  case IBUS_Up:

    return TRUE;

  case IBUS_Down:

    return TRUE;

  case IBUS_BackSpace:
    if (engine->inputed->len)
      {
        g_string_truncate(engine->inputed, (engine->inputed->len) -1);
        return ibus_t9_engine_update(engine);
      }
    return FALSE;
  case IBUS_KP_1:
    engine->inputed = g_string_append_c(engine->inputed, 'h');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_2:
    engine->inputed = g_string_append_c(engine->inputed, 's');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_3:
    engine->inputed = g_string_append_c(engine->inputed, 'p');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_4:
    engine->inputed = g_string_append_c(engine->inputed, 'n');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_5:
    engine->inputed = g_string_append_c(engine->inputed, 'z');
    return ibus_t9_engine_update(engine);
  case IBUS_0 ... IBUS_9:
    return ibus_t9_engine_commit_string(engine, keyval - IBUS_0);
    }

  return FALSE;
}

static void
ibus_t9_engine_focus_in(IBusEngine *engine)
{
  IBusT9Engine * ibus_t9 = IBUS_T9_ENGINE(engine);
  gtk_widget_show_all(ibus_t9->LookupTable);
}

static void
ibus_t9_engine_focus_out(IBusEngine *engine)
{
  IBusT9Engine * ibus_t9 = IBUS_T9_ENGINE(engine);
  gtk_window_get_position(GTK_WINDOW(ibus_t9->LookupTable),
      &ibus_t9->laststate.x, &ibus_t9->laststate.y);
  gtk_widget_hide(ibus_t9->LookupTable);
}
