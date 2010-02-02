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

  rsvg_init();
}

static void
ibus_t9_engine_init(IBusT9Engine *engine)
{
  size_t i;
  IBusT9EngineClass* klass;

  char icon_file[4096];

  GError * err;

  engine->laststate.x = 300;
  engine->laststate.y = 300;
  engine->laststate.width = 200;
  engine->laststate.height = 100;
  engine->drag = 0;

  klass = IBUS_T9_ENGINE_GET_CLASS(engine);

  for (i = 0; i < 5; ++i)
    {
      err = NULL;
      sprintf(icon_file, "%s/key%u.svg", klass->icondir->str, i + 1);
      g_message("load %s \n", icon_file);
      engine->keysicon[i] = rsvg_handle_new_from_file(icon_file, &err);
      if (engine->keysicon[i] == NULL)
        g_error(_("Err opening %s : %s\n"), icon_file, err ? err->message : "");
    }

  engine->LookupTable = gtk_window_new(GTK_WINDOW_POPUP);

  gtk_window_move(GTK_WINDOW(engine->LookupTable), engine->laststate.x,
      engine->laststate.y);
  gtk_window_resize(GTK_WINDOW(engine->LookupTable), engine->laststate.width,
      engine->laststate.height);

  gtk_widget_add_events(GTK_WIDGET(engine->LookupTable), GDK_BUTTON_MOTION_MASK
      | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
  g_signal_connect_after(G_OBJECT(engine->LookupTable),"motion_notify_event",G_CALLBACK(on_mouse_move),engine);
  g_signal_connect(G_OBJECT(engine->LookupTable),"expose-event",G_CALLBACK(on_paint),engine);
  g_signal_connect(G_OBJECT(engine->LookupTable),"button-release-event",G_CALLBACK(on_button),engine);
  g_signal_connect(G_OBJECT(engine->LookupTable),"button-press-event",G_CALLBACK(on_button),engine);

  engine->matched = g_array_sized_new(FALSE, TRUE, sizeof(MATCHED), 20);
  engine->inputed = g_string_new("");
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

static int
ibus_t9_engine_commit_string(IBusT9Engine *engine, guint index)
{
  IBusText *text;
  if (engine->matched->len > index)
    {
      text = ibus_text_new_from_static_string(
          g_array_index(engine->matched,MATCHED,index).hanzi);
      ibus_engine_commit_text((IBusEngine *) engine, text);
      g_object_unref(text);
      g_string_truncate(engine->inputed,0);
      return TRUE;
    }
  return FALSE;
}

static gboolean
ibus_t9_engine_update(IBusT9Engine *engine)
{
  if (engine->inputed->len)
    {
      g_print("input is now %s\n", engine->inputed->str);
      phraser_get_phrases(engine->matched, engine->inputed,
          IBUS_T9_ENGINE_GET_CLASS(engine)->phraser);
    }
  else g_array_set_size(engine->matched,0);
 gdk_window_invalidate_rect(engine->LookupTable->window,0,0);
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
    return ibus_t9_engine_commit_string(engine, 0);
  case IBUS_Return:
    return ibus_t9_engine_commit_string(engine, 0);

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
