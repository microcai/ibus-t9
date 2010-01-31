/* vim:set et sts=4: */
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <sqlite3.h>
#include <ibus.h>
#include <gtk/gtk.h>
#include <librsvg/rsvg.h>


typedef struct _IBusT9Engine IBusT9Engine;
typedef struct _IBusT9EngineClass IBusT9EngineClass;

struct _IBusT9Engine {
	IBusEngine parent;

    /* members */
    GString *preedit;
    gint cursor_pos;

	//  IBusLookupTable *table;
	GtkWidget* LookupTable;
	GdkRectangle laststate;
	GdkPoint lastpoint;
	guint drag;
	RsvgHandle *keysicon[5];
	gboolean iconstate[5];
};

struct _IBusT9EngineClass {
	IBusEngineClass parent;
	sqlite3			* lookupdb;
	GString		*	icondir;
};


#define IBUS_TYPE_T9_ENGINE	\
	(ibus_t9_engine_get_type ())

GType ibus_t9_engine_get_type(void);

#define IBUS_T9_ENGINE_GET_CLASS(obj)	((IBusT9EngineClass*)(IBUS_ENGINE_GET_CLASS(obj)))

#define IBUS_T9_ENGINE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_T9_ENGINE, IBusT9Engine))


typedef struct _LineStoken LineStoken;

struct _LineStoken
{
	int		segments; //包含有的段数目
	GdkPoint* points; //包含的用来构成笔画的点
};

typedef struct _RESULTCHAR RESULTCHAR;

struct _RESULTCHAR{
	gchar	charactor[20];
	gfloat	score;
};

extern char DATAFILE[] ; //= "data/handwriting-zh_CN.model";
extern char icondir[];
#endif
