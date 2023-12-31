/*
 * This file is part of rSON
 * Copyright © 2012-2018 Rachel Mant (dx-mon@users.sourceforge.net)
 *
 * rSON is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rSON is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <crunch.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "rSON.hxx"
using namespace rSON;

#define WANTED_TYPE(whatToDo) \
try \
{ \
	whatToDo; \
} \
catch (const JSONTypeError &e) \
{ \
	fail(e.error()); \
}

#define UNWANTED_TYPE(onWhat, type) \
try \
{ \
	onWhat->as ## type (); \
	fail("Type " #type " converted even though wrong"); \
} \
catch (const JSONTypeError &e) \
{ \
}
