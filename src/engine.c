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
static void	ibus_t9_engine_class_init	(IBusT9EngineClass	*klass);
static void	ibus_t9_engine_init		(IBusT9Engine		*engine);
static void	ibus_t9_engine_destroy		(IBusT9Engine		*engine);
static gboolean
			ibus_t9_engine_process_key_event
                                            (IBusEngine             *engine,
                                             guint               	 keyval,
                                             guint               	 keycode,
                                             guint               	 modifiers);
static void ibus_t9_engine_focus_in    (IBusEngine             *engine);
static void ibus_t9_engine_focus_out   (IBusEngine             *engine);
static void ibus_t9_engine_reset       (IBusEngine             *engine);
static void ibus_t9_engine_enable      (IBusEngine             *engine);
static void ibus_t9_engine_disable     (IBusEngine             *engine);
static void ibus_engine_set_cursor_location (IBusEngine             *engine,
                                             gint                    x,
                                             gint                    y,
                                             gint                    w,
                                             gint                    h);
static void ibus_t9_engine_set_capabilities
                                            (IBusEngine             *engine,
                                             guint                   caps);
static void ibus_t9_engine_page_up     (IBusEngine             *engine);
static void ibus_t9_engine_page_down   (IBusEngine             *engine);
static void ibus_t9_engine_cursor_up   (IBusEngine             *engine);
static void ibus_t9_engine_cursor_down (IBusEngine             *engine);
static void ibus_enchant_property_activate  (IBusEngine             *engine,
                                             const gchar            *prop_name,
                                             gint                    prop_state);
static void ibus_t9_engine_property_show
											(IBusEngine             *engine,
                                             const gchar            *prop_name);
static void ibus_t9_engine_property_hide
											(IBusEngine             *engine,
                                             const gchar            *prop_name);

static void ibus_t9_engine_commit_string
                                            (IBusT9Engine      *enchant,
                                             const gchar            *string);
static void ibus_t9_engine_update      (IBusT9Engine      *enchant);

static IBusEngineClass *parent_class = NULL;

GType
ibus_t9_engine_get_type (void)
{
	static GType type = 0;

	static const GTypeInfo type_info = {
		sizeof (IBusT9EngineClass),
		(GBaseInitFunc)		NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc)	ibus_t9_engine_class_init,
		NULL,
		NULL,
		sizeof (IBusT9Engine),
		0,
		(GInstanceInitFunc)	ibus_t9_engine_init,
	};

	if (type == 0) {
		type = g_type_register_static (IBUS_TYPE_ENGINE,
									   "IBusT9Engine",
									   &type_info,
									   (GTypeFlags) 0);
	}

	return type;
}

static void
ibus_t9_engine_class_init (IBusT9EngineClass *klass)
{
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

	parent_class =  (IBusEngineClass *) g_type_class_peek_parent (klass);

	ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_t9_engine_destroy;

//    engine_class->process_key_event = ibus_t9_engine_process_key_event;

    engine_class->enable = ibus_t9_engine_enable;

    engine_class->disable = ibus_t9_engine_disable;

    engine_class->focus_in = ibus_t9_engine_focus_in;

    engine_class->focus_out = ibus_t9_engine_focus_out;

    klass->icondir = g_string_new(icondir);

    rsvg_init();
}


static void
ibus_t9_engine_init (IBusT9Engine *engine)
{
	size_t i;
	IBusT9EngineClass*	klass;

	char icon_file[4096];

	GError * err;

	engine->laststate.x = 300 ;
	engine->laststate.y = 300 ;
	engine->laststate.width = 200 ;
	engine->laststate.height = 120 ;
	engine->drag = 0;

	klass = IBUS_T9_ENGINE_GET_CLASS(engine);

	for (i = 0; i < 5; ++i) {
		err = NULL;
		sprintf(icon_file,"%s/key%u.svg",klass->icondir->str,i+1);
		g_message("load %s \n",icon_file);
		engine->keysicon[i] = rsvg_handle_new_from_file(icon_file,&err);
		if (engine->keysicon[i] == NULL)
			g_error(_("Err opening %s : %s\n"), icon_file, err ? err->message : "");
	}

//	puts("?????!!!!\n");
//	engine->preedit = g_string_new ("");
//	engine->cursor_pos = 0;

//	ibus_lookup_table_new (9, 0, TRUE, TRUE);
//    ibus_engine_show_lookup_table(IBUS_ENGINE(engine));
}

static void
ibus_t9_engine_destroy (IBusT9Engine *engine)
{
//    if (enchant->preedit) {
//        g_string_free (enchant->preedit, TRUE);
//        enchant->preedit = NULL;
//    }

//    if (enchant->table) {
//        g_object_unref (enchant->table);
//        enchant->table = NULL;
//    }

	ibus_t9_engine_disable(IBUS_ENGINE(engine));


	IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)engine);
}

static void
ibus_t9_engine_update_lookup_table (IBusT9Engine *enchant)
{
    gchar ** sugs;
    gint n_sug, i;
    gboolean retval;

    if (enchant->preedit->len == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) enchant);
        return;
    }
//
//    ibus_lookup_table_clear (enchant->table);
//
//    if (sugs == NULL || n_sug == 0) {
//        ibus_engine_hide_lookup_table ((IBusEngine *) enchant);
//        return;
//    }
//
//    for (i = 0; i < n_sug; i++) {
//        ibus_lookup_table_append_candidate (enchant->table, ibus_text_new_from_string (sugs[i]));
//    }
//
//    ibus_engine_update_lookup_table ((IBusEngine *) enchant, enchant->table, TRUE);

}

static void
ibus_t9_engine_update_preedit (IBusT9Engine *enchant)
{
    IBusText *text;
    gint retval;

    text = ibus_text_new_from_static_string (enchant->preedit->str);
    text->attrs = ibus_attr_list_new ();

    ibus_attr_list_append (text->attrs,
                           ibus_attr_underline_new (IBUS_ATTR_UNDERLINE_SINGLE, 0, enchant->preedit->len));

    if (enchant->preedit->len > 0) {
 //       retval = enchant_dict_check (dict, enchant->preedit->str, enchant->preedit->len);
        if (retval != 0) {
            ibus_attr_list_append (text->attrs,
                               ibus_attr_foreground_new (0xff0000, 0, enchant->preedit->len));
        }
    }

    ibus_engine_update_preedit_text ((IBusEngine *)enchant,
                                     text,
                                     enchant->cursor_pos,
                                     TRUE);
    g_object_unref (text);

}

/* commit preedit to client and update preedit */
static gboolean
ibus_t9_engine_commit_preedit (IBusT9Engine *enchant)
{
    if (enchant->preedit->len == 0)
        return FALSE;

    ibus_t9_engine_commit_string (enchant, enchant->preedit->str);
    g_string_assign (enchant->preedit, "");
    enchant->cursor_pos = 0;

    ibus_t9_engine_update (enchant);

    return TRUE;
}


static void
ibus_t9_engine_commit_string (IBusT9Engine *enchant,
                                   const gchar       *string)
{
    IBusText *text;
    text = ibus_text_new_from_static_string (string);
    ibus_engine_commit_text ((IBusEngine *)enchant, text);
    g_object_unref (text);
}

static void
ibus_t9_engine_update (IBusT9Engine *enchant)
{
    ibus_t9_engine_update_preedit (enchant);
    ibus_engine_hide_lookup_table ((IBusEngine *)enchant);
}

#define is_alpha(c) (((c) >= IBUS_a && (c) <= IBUS_z) || ((c) >= IBUS_A && (c) <= IBUS_Z))

static gboolean
ibus_t9_engine_process_key_event (IBusEngine *engine,
                                       guint       keyval,
                                       guint       keycode,
                                       guint       modifiers)
{
    IBusText *text;
    IBusT9Engine *enchant = (IBusT9Engine *)engine;

    if (modifiers & IBUS_RELEASE_MASK)
        return FALSE;

    modifiers &= (IBUS_CONTROL_MASK | IBUS_MOD1_MASK);

    if (modifiers == IBUS_CONTROL_MASK && keyval == IBUS_s) {
        ibus_t9_engine_update_lookup_table (enchant);
        return TRUE;
    }

    if (modifiers != 0) {
        if (enchant->preedit->len == 0)
            return FALSE;
        else
            return TRUE;
    }


    switch (keyval) {
    case IBUS_space:
        g_string_append (enchant->preedit, " ");
        return ibus_t9_engine_commit_preedit (enchant);
    case IBUS_Return:
        return ibus_t9_engine_commit_preedit (enchant);

    case IBUS_Escape:
        if (enchant->preedit->len == 0)
            return FALSE;

        g_string_assign (enchant->preedit, "");
        enchant->cursor_pos = 0;
        ibus_t9_engine_update (enchant);
        return TRUE;

    case IBUS_Left:
        if (enchant->preedit->len == 0)
            return FALSE;
        if (enchant->cursor_pos > 0) {
            enchant->cursor_pos --;
            ibus_t9_engine_update (enchant);
        }
        return TRUE;

    case IBUS_Right:
        if (enchant->preedit->len == 0)
            return FALSE;
        if (enchant->cursor_pos < enchant->preedit->len) {
            enchant->cursor_pos ++;
            ibus_t9_engine_update (enchant);
        }
        return TRUE;

    case IBUS_Up:
        if (enchant->preedit->len == 0)
            return FALSE;
        if (enchant->cursor_pos != 0) {
            enchant->cursor_pos = 0;
            ibus_t9_engine_update (enchant);
        }
        return TRUE;

    case IBUS_Down:
        if (enchant->preedit->len == 0)
            return FALSE;

        if (enchant->cursor_pos != enchant->preedit->len) {
            enchant->cursor_pos = enchant->preedit->len;
            ibus_t9_engine_update (enchant);
        }

        return TRUE;

    case IBUS_BackSpace:
        if (enchant->preedit->len == 0)
            return FALSE;
        if (enchant->cursor_pos > 0) {
            enchant->cursor_pos --;
            g_string_erase (enchant->preedit, enchant->cursor_pos, 1);
            ibus_t9_engine_update (enchant);
        }
        return TRUE;

    case IBUS_Delete:
        if (enchant->preedit->len == 0)
            return FALSE;
        if (enchant->cursor_pos < enchant->preedit->len) {
            g_string_erase (enchant->preedit, enchant->cursor_pos, 1);
            ibus_t9_engine_update (enchant);
        }
        return TRUE;
    }

    if (is_alpha (keyval)) {
        g_string_insert_c (enchant->preedit,
                           enchant->cursor_pos,
                           keyval);

        enchant->cursor_pos ++;
        ibus_t9_engine_update (enchant);

        return TRUE;
    }

    return FALSE;
}

static void ibus_t9_engine_enable      (IBusEngine             *engine)
{
	IBusT9Engine	* ibus_t9 = IBUS_T9_ENGINE(engine);

	if(!ibus_t9->LookupTable)
		ibus_t9->LookupTable = gtk_window_new(GTK_WINDOW_POPUP);

	gtk_window_move(GTK_WINDOW(ibus_t9->LookupTable),ibus_t9->laststate.x,ibus_t9->laststate.y);
	gtk_window_resize(GTK_WINDOW(ibus_t9->LookupTable),ibus_t9->laststate.width,ibus_t9->laststate.height);

	gtk_widget_add_events(GTK_WIDGET(ibus_t9->LookupTable),
			GDK_BUTTON_MOTION_MASK | GDK_BUTTON_RELEASE_MASK| GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK);
	g_signal_connect_after(G_OBJECT(ibus_t9->LookupTable),"motion_notify_event",G_CALLBACK(on_mouse_move),engine);
	g_signal_connect(G_OBJECT(ibus_t9->LookupTable),"expose-event",G_CALLBACK(on_paint),engine);
	g_signal_connect(G_OBJECT(ibus_t9->LookupTable),"button-release-event",G_CALLBACK(on_button),engine);
	g_signal_connect(G_OBJECT(ibus_t9->LookupTable),"button-press-event",G_CALLBACK(on_button),engine);

}

static void ibus_t9_engine_disable     (IBusEngine             *engine)
{
	IBusT9Engine	* ibus_t9 = IBUS_T9_ENGINE(engine);
//	gtk_window_get_position(GTK_WINDOW(ibus_t9->LookupTable),&ibus_t9->laststate.x,&ibus_t9->laststate.y);
	g_print("%s , %d %d\n",__func__,ibus_t9->laststate.x,ibus_t9->laststate.y);
	gtk_widget_destroy(ibus_t9->LookupTable);
	ibus_t9->LookupTable = NULL;
}

static void ibus_t9_engine_focus_in    (IBusEngine             *engine)
{
	IBusT9Engine	* ibus_t9 = IBUS_T9_ENGINE(engine);
	gtk_widget_show_all(ibus_t9->LookupTable);
}

static void ibus_t9_engine_focus_out   (IBusEngine             *engine)
{
	IBusT9Engine	* ibus_t9 = IBUS_T9_ENGINE(engine);
	gtk_window_get_position(GTK_WINDOW(ibus_t9->LookupTable),&ibus_t9->laststate.x,&ibus_t9->laststate.y);
	gtk_widget_hide(ibus_t9->LookupTable);
}
