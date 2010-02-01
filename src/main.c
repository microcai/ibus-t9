/* vim:set et sts=4: */
#include <config.h>
#include <unistd.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "prase.h"

static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

static void ibus_disconnected_cb(IBusBus *bus, gpointer user_data)
{
	gtk_main_quit();
}

char DATAFILE[1024] = PKGDATADIR"/tables/table.txt";
static int have_ibus;
char icondir[4096] = PKGDATADIR"/icons/";
struct parameter_tags paramters[] =
{
{ "--ibus", (const char*) &have_ibus, NULL, sizeof(have_ibus), 6, BOOL_both },
{ "--icon", (const char*) icondir, "--icon the icon dir", sizeof(icondir), 6,	STRING },
{ "--table", (const char*) DATAFILE, "--table the table file", sizeof(DATAFILE), 7,	STRING },
{ 0 } };

static void init_inside(const char *exefile)
{
	IBusComponent *component;

	ibus_init();

	bus = ibus_bus_new();
	g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

	factory = ibus_factory_new(ibus_bus_get_connection(bus));

	ibus_bus_request_name(bus, "org.freedesktop.IBus.T9", 0);

        component = ibus_component_new("org.freedesktop.IBus.T9",
                        _("T9 input method"), PACKAGE_VERSION, "GPL", MICROCAI_WITHEMAIL, PACKAGE_BUGREPORT,
                        exefile, GETTEXT_PACKAGE);

	ibus_bus_register_component(bus, component);

	ibus_factory_add_engine(factory, "T9", IBUS_TYPE_T9_ENGINE);

	g_object_unref(component);
}

static void init_outside(const char * icon_dir, const char *exefile)
{
	char iconfile [1024];
	strcpy(iconfile,icon_dir);
	strcat(iconfile,"/ibus-t9.svg");

	IBusComponent *component;
	IBusEngineDesc * desc;

	ibus_init();

	bus = ibus_bus_new();
	g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

	factory = ibus_factory_new(ibus_bus_get_connection(bus));

	ibus_bus_request_name(bus, "org.freedesktop.IBus.T9", 0);



	desc = ibus_engine_desc_new("T9", "ibus-T9",
			_("T9 input method"), "zh_CN", "GPL",
			MICROCAI_WITHEMAIL, iconfile, "us");

	component = ibus_component_new("org.freedesktop.IBus.T9",
			_("T9 input method"), PACKAGE_VERSION, "GPL", MICROCAI_WITHEMAIL, PACKAGE_BUGREPORT,
			exefile, GETTEXT_PACKAGE);

	ibus_component_add_engine(component, desc);

	ibus_bus_register_component(bus, component);

	ibus_factory_add_engine(factory, "T9", IBUS_TYPE_T9_ENGINE);

	g_object_unref(component);
}

int main(int argc, char* argv[])
{
  gtk_init(&argc, &argv);
  ibus_init();
  ParseParameters(&argc, &argv, paramters);
  setlocale(LC_ALL, "");
  gtk_set_locale();
  textdomain(GETTEXT_PACKAGE);
  bindtextdomain(GETTEXT_PACKAGE, PREFIX"/share/locale");
  char exefile[4096] =
    { 0 };

  if (!have_ibus)
    {
      char iconfile[4096] =
        { 0 };
      init_outside(realpath(icondir, iconfile), realpath(argv[0], exefile));
      printf(_("ibus-t9 Version %s Start Up\n"), PACKAGE_VERSION);
    }
  else
    {
      init_inside(realpath(argv[0], exefile));
    }
  gtk_main();
  return 0;
}
