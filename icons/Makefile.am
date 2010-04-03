# vim:set et ts=4:
#
# ibus-t9 - The t9 engine for IBus
#
# Copyright (c) 2007-2008 Yu Yuwei <acevery@gmail.com>
# Copyright (c) 2010-2010 microcai  <microcai@fedoraproject.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

ICON_PREFIX = ibus_t9_icon

icons_DATA = \
	ibus-t9.svg \
	full-letter.svg \
	half-letter.svg \
	full-punct.svg \
	half-punct.svg \
    key1.svg     key2.svg     key3.svg     key4.svg     key5.svg \
	$(NULL)

iconsdir = $(pkgdatadir)/icons

EXTRA_DIST = \
	$(icons_DATA) \
	$(NULL)

DISTCLEANFILES = ibus_t9.icons.h

all : ibus_t9.icons.h

ibus_t9.icons.h : $(icons_DATA)
	gdk-pixbuf-csource --raw --static --build-list \
		$(ICON_PREFIX)_key1 $(srcdir)/key1.svg  \
		$(ICON_PREFIX)_key2 $(srcdir)/key2.svg  \
		$(ICON_PREFIX)_key3 $(srcdir)/key3.svg  \
		$(ICON_PREFIX)_key4 $(srcdir)/key4.svg \
		$(ICON_PREFIX)_key5 $(srcdir)/key5.svg  > $@	