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
static IBusFactory *factory = NULL;

const gchar * datafile = PKGDATADIR"/tables/table.txt";
static int have_ibus;
const char * icondir = PKGDATADIR"/icons/";

static void init_inside(const char *exefile)
{
	IBusComponent *component;

	bus = ibus_bus_new();
	g_signal_connect (bus, "disconnected", G_CALLBACK (gtk_main_quit), NULL);

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
	char * iconfile;

	iconfile = g_strdup_printf("%s/ibus-t9.svg",icon_dir);

	IBusComponent *component;
	IBusEngineDesc * desc;


	bus = ibus_bus_new();
	g_signal_connect (bus, "disconnected", G_CALLBACK (gtk_main_quit), NULL);

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

	g_free(iconfile);

	iconfile = g_malloc(1024);

	realpath(icondir,iconfile);

	icondir = iconfile;
}

int main(int argc, char* argv[])
{
	int i;
	const gchar * locale_dir = NULL;

	setlocale(LC_ALL, "");
	gtk_set_locale();
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

	g_assert(gtk_init_with_args(&argc, &argv, GETTEXT_PACKAGE,paramters, GETTEXT_PACKAGE, NULL));

	if(locale_dir)
	{
		bindtextdomain(GETTEXT_PACKAGE, locale_dir);
	}

	ibus_init();

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
