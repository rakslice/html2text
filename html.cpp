/*
 * Portions Copyright (c) 1999 GMRS Software GmbH
 * Carl-von-Linde-Str. 38, D-85716 Unterschleissheim, http://www.gmrs.de
 * All rights reserved.
 *
 * Author: Arno Unkrig <arno@unkrig.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License in the file COPYING for more details.
 */

#include <stdlib.h>
#include <stdarg.h>

#include "html.h"
#include "HTMLParser.tab.hh"
#include "cmp_nocase.h"
#include "iconvstream.h"

/*
 * Some C++ compilers (e.g. EGCS 2.91.66) have problems if all virtual
 * methods of a class are inline or pure virtual, so we define the virtual
 * "Element::~Element()", which is the only virtual method, non-inline,
 * although it is empty.
 */
Element::~Element()
{}

istr
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	const char               *dflt
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0)
				return (*i).second;
		}
	}
	return istr(dflt);
}

// *exists is set to false if attribute *name does not exist - Johannes Geiger
istr
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	bool                     *exists
	)
{
	*exists = true;
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0)
				return (*i).second;
		}
	}
	*exists = false;
	return istr("");
}

int
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	int dflt
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0)
				return atoi((*i).second.tostring().c_str());
		}
	}
	return dflt;
}

int
get_attribute(
	const list<TagAttribute> *as,
	const char               *name,
	int dflt,
	const char               *s1,
	int v1,
	...
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0) {
				const char *s = s1;
				int v = v1;

				va_list va;
				va_start(va, v1);
				for (;;) {
					if (cmp_nocase(s, (*i).second.tostring().c_str()) == 0)
						break;
					s = va_arg(va, const char *);
					if (!s) {
						v = dflt;
						break;
					}
					v = va_arg(va, int);
				}
				va_end(va);
				return v;
			}
		}
	}
	return dflt;
}

int
get_attribute(
	const list<TagAttribute> *as,
	const char               *name, // Attribute name
	const char               *dflt1,// If attribute not specified
	int dflt2,                      // If string value does not match s1, ...
	const char               *s1,
	int v1,
	...
	)
{
	if (as) {
		list<TagAttribute>::const_iterator i;
		for (i = as->begin(); i != as->end(); ++i) {
			if (cmp_nocase((*i).first, name) == 0) {
				dflt1 = (*i).second.tostring().c_str();
				break;
			}
		}
	}

	if (!dflt1)
		return dflt2;

	const char *s = s1;
	int v = v1;

	va_list va;
	va_start(va, v1);
	for (;;) {
		if (cmp_nocase(s, dflt1) == 0)
			break;
		s = va_arg(va, const char *);
		if (!s)
			break;
		v = va_arg(va, int);
	}
	va_end(va);

	return s ? v : dflt2;
}

istr
get_style_attr(istr *style, const char *name, const char *dflt)
{
	bool iskey = true;
	size_t keystart = 0;
	size_t valstart = 0;
	size_t t;

	if (style != NULL) {
		for (size_t i = 0; i < style->length(); i++) {
			switch ((*style)[i]) {
				case ':':
					/* keystart:i = key */
					for (t = i - 1; (*style)[t] == ' '; t--)
						;
					t++;
					iskey = false;
					valstart = i + 1;
					break;
				case ';':
					/* end of value */
					if (!iskey) {
						if (style->compare(keystart, t - keystart, name) == 0) {
							for (t = i - 1; (*style)[t] == ' '; t--)
								;
							t++;
							return style->slice(valstart, t - valstart);
						}
					}
					keystart = i + 1;
					iskey = true;
					break;
				case ' ':
					if (keystart == i)
						keystart++;
					if (valstart == i)
						valstart++;
					break;
			}
		}
	}

	return istr(dflt);
}
