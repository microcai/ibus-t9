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
ibus_t9_engine_enable(IBusEngine *engine);

static void
ibus_t9_engine_disable(IBusEngine *engine);

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
ibus_t9_property_activate(IBusEngine *engine, const gchar *prop_name,guint prop_state);
static void
ibus_t9_candidate_clicked(IBusEngine *engine, guint index, guint button,guint state);
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

  engine_class->enable = ibus_t9_engine_enable;

//  engine_class->disable = ibus_t9_engine_disable;
//
  engine_class->focus_in = ibus_t9_engine_focus_in;
//
//  engine_class->focus_out = ibus_t9_engine_focus_out;

  engine_class->property_activate = ibus_t9_property_activate;

  engine_class->candidate_clicked = ibus_t9_candidate_clicked;

  klass->icondir = g_string_new(icondir);

  if((klass->phraser = phraser_new(datafile)))
      phraser_optimise(klass->phraser);
  else g_error(_("cannot open %s"),datafile);

  klass->commit_string = ibus_t9_engine_commit_string;
}

static void
ibus_t9_engine_init(IBusT9Engine *engine)
{
  size_t i;
  IBusT9EngineClass* klass;
  GdkPixbuf * px;

  GError * err;

  klass = IBUS_T9_ENGINE_GET_CLASS(engine);

  engine->matched = g_array_sized_new(FALSE, TRUE, sizeof(MATCHED), 20);
  engine->inputed = g_string_new("");

  return;
}

static void
ibus_t9_engine_destroy(IBusT9Engine *engine)
{
  g_print("%s\n", __func__);//, engine->laststate.x, engine->laststate.y);
  IBUS_OBJECT_CLASS(g_type_class_peek_parent(IBUS_ENGINE_GET_CLASS(engine)))->destroy(IBUS_OBJECT(engine));
}

static gboolean
ibus_t9_engine_update(IBusT9Engine *engine)
{
	int i;

	if (engine->inputed->len)
	{
		g_print("input is now %s\n", engine->inputed->str);
		phraser_get_phrases(engine->matched, engine->inputed,
				IBUS_T9_ENGINE_GET_CLASS(engine)->phraser);
	}
	else
	{
		engine->matched = g_array_set_size(engine->matched, 0);
		ibus_engine_hide_preedit_text(IBUS_ENGINE(engine));
		ibus_engine_hide_auxiliary_text(IBUS_ENGINE(engine));
		ibus_engine_hide_lookup_table(IBUS_ENGINE(engine));
		return FALSE;
	}

	engine->table = ibus_lookup_table_new(10,0,0,1);

//	ibus_engine_update_preedit_text(IBUS_ENGINE(engine),ibus_text_new_from_static_string(engine->inputed->str),0,1);

	for (i = 0; i < MIN(engine->matched->len,10); ++i)
	{
		gchar * hanzi = g_array_index(engine->matched,MATCHED,i).hanzi;

		ibus_lookup_table_append_candidate(engine->table,ibus_text_new_from_static_string(hanzi));
	}

	ibus_engine_update_lookup_table(IBUS_ENGINE(engine),engine->table,1);

	ibus_engine_update_auxiliary_text(IBUS_ENGINE(engine),ibus_text_new_from_static_string(engine->inputed->str),1);

	ibus_engine_update_preedit_text(IBUS_ENGINE(engine),ibus_text_new_from_static_string(g_array_index(engine->matched,MATCHED,0).hanzi),0,1);

	return TRUE;
}

static int ibus_t9_engine_commit_string(IBusT9Engine *engine, guint index)
{
	if (engine->matched->len > index)
	{
		ibus_engine_commit_text((IBusEngine *) engine,
				ibus_text_new_from_static_string(
						g_array_index(engine->matched,MATCHED,index).hanzi));
		g_string_truncate(engine->inputed, 0);
		ibus_t9_engine_update(engine);
		return TRUE;
	}
	return FALSE;
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
  case IBUS_h:
    engine->inputed = g_string_append_c(engine->inputed, 'h');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_2:
  case IBUS_s:
    engine->inputed = g_string_append_c(engine->inputed, 's');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_3:
  case IBUS_p:
    engine->inputed = g_string_append_c(engine->inputed, 'p');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_4:
  case IBUS_n:
    engine->inputed = g_string_append_c(engine->inputed, 'n');
    return ibus_t9_engine_update(engine);
  case IBUS_KP_5:
  case IBUS_z:
    engine->inputed = g_string_append_c(engine->inputed, 'z');
    return ibus_t9_engine_update(engine);
  case IBUS_0 ... IBUS_9:
    return ibus_t9_engine_commit_string(engine, keyval - IBUS_0);
    }

  return FALSE;
}
static void
ibus_t9_engine_enable(IBusEngine *engine)
{
//	((IBusT9Engine*)engine)->table = ibus_lookup_table_new(10,0,0,1);
//	ibus_engine_update_lookup_table(engine,((IBusT9Engine*)engine)->table,1);
	ibus_t9_engine_update((IBusT9Engine*)engine);
}

static void
ibus_t9_engine_disable(IBusEngine *engine)
{
//	((IBusT9Engine*)engine)->table = NULL;
	ibus_engine_hide_lookup_table(engine);
}

static void
ibus_t9_engine_focus_in(IBusEngine *engine)
{
	int i;
	IBusProperty * p;

	IBusT9Engine * ibus_t9 = IBUS_T9_ENGINE(engine);
//	gtk_widget_show_all(ibus_t9->LookupTable);

	IBusPropList * pl = ibus_prop_list_new();

	for(i=1;i<=5;i++)
	{
		gchar * icon_name = g_strdup_printf("%s/key%d.svg",icondir,i);

		gchar * prog_id = g_strdup_printf("%d",i);


		p= ibus_property_new(prog_id, PROP_TYPE_NORMAL,
				ibus_text_new_from_static_string(_("key")),
				icon_name, ibus_text_new_from_static_string(
						_("click to input")), TRUE, TRUE,
				PROP_STATE_UNCHECKED, NULL);

		ibus_prop_list_append(pl, p);
		g_free(prog_id);
		g_free(icon_name);
	}

	ibus_engine_register_properties(IBUS_ENGINE(engine), pl);
}

static void
ibus_t9_engine_focus_out(IBusEngine *engine)
{
  IBusT9Engine * ibus_t9 = IBUS_T9_ENGINE(engine);
//  gtk_window_get_position(GTK_WINDOW(ibus_t9->LookupTable),
//      &ibus_t9->laststate.x, &ibus_t9->laststate.y);
//  gtk_widget_hide(ibus_t9->LookupTable);
}

static void
ibus_t9_property_activate(IBusEngine *engine, const gchar *prop_name,
    guint prop_state)
{
	ibus_t9_engine_process_key_event;

	char inputs_key_map [] = { 'h', 's', 'p','n','z'};

	char  input;

	switch(*prop_name)
	{
	case '1' ... '5':
		input= inputs_key_map[*prop_name-'1'];

	    IBUS_T9_ENGINE(engine)->inputed = g_string_append_c(IBUS_T9_ENGINE(engine)->inputed, input);

	    ibus_t9_engine_update(IBUS_T9_ENGINE(engine));

		g_print("press %c\n",input);
		break;
	}
}

static void
ibus_t9_candidate_clicked(IBusEngine *engine, guint index, guint button,guint state)
{
	ibus_engine_hide_preedit_text(IBUS_ENGINE(engine));
	ibus_engine_commit_text(engine,ibus_text_new_from_static_string(g_array_index(IBUS_T9_ENGINE(engine)->matched,MATCHED,index).hanzi));
}
