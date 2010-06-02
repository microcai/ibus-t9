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

IBusBus *bus = NULL;

const gchar * datafile = PKGDATADIR"/tables/table.txt";
const char * icondir = PKGDATADIR"/icons/";

int main(int argc, char* argv[])
{
	int have_ibus,i;
	const gchar * locale_dir = NULL;
	IBusComponent *component;
	IBusFactory *factory = NULL;

	setlocale(LC_ALL, "");
	textdomain(GETTEXT_PACKAGE);

	GOptionEntry paramters[] =
	{
		{"ibus",'\0',0,G_OPTION_ARG_NONE,&have_ibus},
		{"icon",'\0',0,G_OPTION_ARG_STRING,&icondir,_("the icon file"),N_("icon file")},
		{"table",'\0',0,G_OPTION_ARG_STRING,&datafile,_("set table file path"),N_("tablefile")},
		{"locale",'\0',0,G_OPTION_ARG_STRING,&locale_dir,_("set locale path"),N_("locale")},
		{0}
	};

	ibus_init();

	GOptionContext * context = g_option_context_new("");

	g_option_context_add_main_entries(context,paramters,GETTEXT_PACKAGE);

	g_assert(g_option_context_parse(context,&argc,&argv,NULL));

	g_option_context_free(context);

	if(locale_dir)
	{
		bindtextdomain(GETTEXT_PACKAGE, locale_dir);
	}

	bus = ibus_bus_new();

	g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_quit), NULL);

	factory = ibus_factory_new(ibus_bus_get_connection(bus));

	ibus_bus_request_name(bus, "org.freedesktop.IBus.T9", 0);

	component = ibus_component_new("org.freedesktop.IBus.T9",
                        _("T9 input method"), PACKAGE_VERSION, "GPL", MICROCAI_WITHEMAIL, PACKAGE_BUGREPORT,
                        argv[0], GETTEXT_PACKAGE);

	if (!have_ibus)
	{
		icondir = realpath(icondir,0);

		char * iconfile = g_strdup_printf("%s/ibus-t9.svg",icondir);

		IBusEngineDesc * desc = ibus_engine_desc_new("T9", "ibus-T9",
				_("T9 input method"), "zh_CN", "GPL",
				MICROCAI_WITHEMAIL, iconfile, "us");

		g_free(iconfile);

		ibus_component_add_engine(component, desc);
	}

	ibus_bus_register_component(bus, component);

	ibus_factory_add_engine(factory, "T9", IBUS_TYPE_T9_ENGINE);

	g_object_unref(component);

	printf(_("ibus-t9 Version %s Start Up\n"), PACKAGE_VERSION);

	ibus_main();
	return 0;
}
