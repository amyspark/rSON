/*
 * This file is part of rSON
 * Copyright © 2017 Rachel Mant (dx-mon@users.sourceforge.net)
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

void tryParserErrorOk(const JSONParserErrorType error)
{
	const JSONParserError err{error};
	assertNotNull(err.error());
}

void testParserError()
{
	assertIntEqual(JSONParserError(JSON_PARSER_EOF).errorType(), JSON_PARSER_EOF);
	tryParserErrorOk(JSON_PARSER_EOF);
	tryParserErrorOk(JSON_PARSER_BAD_JSON);
	tryParserErrorOk(JSON_PARSER_BAD_FILE);

	const JSONParserError err{static_cast<JSONParserErrorType>(-1)};
	assertNotNull(err.what());
	assertStringEqual(err.what(), "Invalid unknown error type for parser error");
}

void tryTypeErrorOk(const JSONAtomType type)
{
	const JSONTypeError err{type, type};
	assertNotNull(err.error());
}

void testTypeError()
{
	tryTypeErrorOk(JSON_TYPE_NULL);
	tryTypeErrorOk(JSON_TYPE_BOOL);
	tryTypeErrorOk(JSON_TYPE_INT);
	tryTypeErrorOk(JSON_TYPE_FLOAT);
	tryTypeErrorOk(JSON_TYPE_STRING);
	tryTypeErrorOk(JSON_TYPE_OBJECT);
	tryTypeErrorOk(JSON_TYPE_ARRAY);

	const auto badType{static_cast<JSONAtomType>(-1)};
	const JSONTypeError err{badType, badType};
	assertNotNull(err.what());
	assertStringEqual(err.what(), "Expecting unknown, found unknown");
}

void testObjectError()
{
	[]()
	{
		const JSONObjectError err{JSON_OBJECT_BAD_KEY};
		assertNotNull(err.error());
	}();

	[]()
	{
		const JSONObjectError err{static_cast<JSONObjectErrorType>(-1)};
		assertNotNull(err.what());
		assertStringEqual(err.what(), "Invalid unknown error type for object error");
	}();
}

void tryArrayErrorOk(const JSONArrayErrorType type)
{
	const JSONArrayError err{type};
	assertNotNull(err.error());
}

void testArrayError()
{
	tryArrayErrorOk(JSON_ARRAY_OOB);
	tryArrayErrorOk(JSON_ARRAY_BAD_ATOM);

	const JSONArrayError err{static_cast<JSONArrayErrorType>(-1)};
	assertNotNull(err.what());
	assertStringEqual(err.what(), "Invalid unknown error type for array error");
}

extern "C"
{
BEGIN_REGISTER_TESTS()
	TEST(testParserError)
	TEST(testTypeError)
	TEST(testObjectError)
	TEST(testArrayError)
END_REGISTER_TESTS()
}
