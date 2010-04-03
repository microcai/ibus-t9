/* vim:set et sts=4: */
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>
#include <gtk/gtk.h>

typedef struct _PHRASER PHRASER;
typedef struct _IBusT9Engine IBusT9Engine;
typedef struct _IBusT9EngineClass IBusT9EngineClass;

typedef struct _MATCHED MATCHED;

struct _MATCHED{
  char  code[64-16];
  char  hanzi[16];
};

struct button_data
{
	IBusT9Engine * engine;
	int index;
};

struct _IBusT9Engine
{
  IBusEngine parent;

  /* members */

  //  IBusLookupTable *table;
  GtkWidget* LookupTable;
  GtkWidget* tables;
  GtkWidget* box;
  GdkRectangle laststate;
  GdkPoint lastpoint;
  guint drag;
  GdkPixbuf *keysicon[5];
  gboolean iconstate[5];

  GString * inputed;
  GArray * matched;
  guint    page;

  struct button_data	stok_botton_call_back[5];
  struct button_data	table_botton_call_back[10];
};

struct _IBusT9EngineClass
{
  IBusEngineClass parent;
  GString * icondir;
  PHRASER * phraser;
  int  (*commit_string)(IBusT9Engine *engine, guint index);
};

#define IBUS_TYPE_T9_ENGINE	\
	(ibus_t9_engine_get_type ())

GType
ibus_t9_engine_get_type(void);

#define IBUS_T9_ENGINE_GET_CLASS(obj)	((IBusT9EngineClass*)(IBUS_ENGINE_GET_CLASS(obj)))

#define IBUS_T9_ENGINE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_T9_ENGINE, IBusT9Engine))

struct _PHRASER
{
  char * filename;
  char * start_ptr; // for nmaped access
  size_t fsize; //for nmaped access
};
PHRASER * phraser_new(char * file);
void phraser_free(PHRASER*phraser);
int phraser_optimise(PHRASER * phraser);
int phraser_get_phrases(GArray * result, GString * input, PHRASER * phraser);

extern char DATAFILE[]; //= "data/handwriting-zh_CN.model";
extern char icondir[];
#endif
