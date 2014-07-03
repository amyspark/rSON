/*
 * This file is part of rSON
 * Copyright © 2012-2013 Rachel Mant (dx-mon@users.sourceforge.net)
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

#include "test.h"

char *strnew(const char *str)
{
	char *ret = new char[strlen(str) + 1];
	strcpy(ret, str);
	return ret;
}

void doTest(JSONAtom *atom, const char *result)
{
	char *json = writeJSON(atom);
	assertNotNull(json);
	assertStringEqual(json, result);
	freeString(&json);
	assertNull(json);
}

void testNull()
{
	JSONNull *null = new JSONNull();
	doTest(null, "null");
	delete null;
}

void testBool()
{
	JSONBool *bTrue = new JSONBool(true);
	JSONBool *bFalse = new JSONBool(false);
	doTest(bTrue, "true");
	doTest(bFalse, "false");
	delete bTrue;
	delete bFalse;
}

void testInt()
{
	JSONInt *num = new JSONInt(8192);
	doTest(num, "8192");
	delete num;
}

void testFloat()
{
	JSONFloat *num = new JSONFloat(8192.016384);
	doTest(num, "8192.016384");
	delete num;
}

void testObject()
{
	char *key;
	JSONObject *obj;

	obj = new JSONObject();
	doTest(obj, "{}");

	key = strnew("test");
	obj->add(key, new JSONNull());
	doTest(obj, "{\"test\": null}");

	key = strnew("array");
	obj->add(key, new JSONArray());
	doTest(obj, "{\"array\": [], \"test\": null}");

	delete obj;
}

void testArray()
{
	JSONArray *arr;
	arr = new JSONArray();
	doTest(arr, "[]");
	arr->add(new JSONNull());
	doTest(arr, "[null]");
	arr->add(new JSONBool(true));
	doTest(arr, "[null, true]");
	delete arr;
}

#ifdef __cplusplus
extern "C"
{
#endif

BEGIN_REGISTER_TESTS()
	TEST(testNull)
	TEST(testBool)
	TEST(testInt)
	TEST(testFloat)
	TEST(testObject)
	TEST(testArray)
END_REGISTER_TESTS()

#ifdef __cplusplus
}
#endif