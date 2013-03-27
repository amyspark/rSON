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

#ifndef __rSON_H__
#define __rSON_H__

#include <inttypes.h>
#include <map>
#include <vector>
#include <string.h>

#ifdef _WINDOWS
	#ifdef __FC__
		#define rSON_API __declspec(dllexport)
	#else
		#define rSON_API __declspec(dllimport)
	#endif
	#define rSON_CLS_API	rSON_API
#else
	#if __GNUC__ >= 4
		#define DEFAULT_VISIBILITY __attribute__ ((visibility("default")))
	#else
		#define DEFAULT_VISIBILITY
	#endif
	#define rSON_CLS_API DEFAULT_VISIBILITY
	#define rSON_API extern rSON_CLS_API
#endif

namespace rSON
{
	// Enumerations

	typedef enum JSONAtomType
	{
		JSON_TYPE_NULL,
		JSON_TYPE_BOOL,
		JSON_TYPE_INT,
		JSON_TYPE_FLOAT,
		JSON_TYPE_STRING,
		JSON_TYPE_OBJECT,
		JSON_TYPE_ARRAY
	} JSONAtomType;

	typedef enum JSONParserErrorType
	{
		JSON_PARSER_EOF,
		JSON_PARSER_BAD_JSON
	} JSONParserErrorType;

	typedef enum JSONObjectErrorType
	{
		JSON_OBJECT_BAD_KEY
	} JSONObjectErrorType;

	typedef enum JSONArrayErrorType
	{
		JSON_ARRAY_OOB
	} JSONArrayErrorType;

	// Exception classes

	class rSON_CLS_API JSONParserError
	{
	private:
		JSONParserErrorType parserError;

	public:
		JSONParserError(JSONParserErrorType errorType);
		~JSONParserError();
		JSONParserErrorType errorType() const;
		const char *error() const;
	};

	class rSON_CLS_API JSONTypeError
	{
	private:
		char *errorStr;
		const char *typeToString(JSONAtomType type) const;

	public:
		JSONTypeError(JSONAtomType actual, JSONAtomType expected);
		~JSONTypeError();
		const char *error() const;
	};

	class rSON_CLS_API JSONObjectError
	{
	private:
		JSONObjectErrorType objectError;

	public:
		JSONObjectError(JSONObjectErrorType errorType);
		~JSONObjectError();
		const char *error() const;
	};

	class rSON_CLS_API JSONArrayError
	{
	private:
		JSONArrayErrorType arrayError;

	public:
		JSONArrayError(JSONArrayErrorType errorType);
		~JSONArrayError();
		const char *error() const;
	};

	// Support types
	struct StringLess
	{
		inline bool operator()(char *x, char *y) const
		{
			return strcmp(x, y) < 0;
		}
	};

	// Hierachy types

	class JSONObject;
	class JSONArray;

	class rSON_CLS_API JSONAtom
	{
	private:
		JSONAtomType type;

	protected:
		JSONAtom();
		JSONAtom(JSONAtomType type);

	public:
		virtual ~JSONAtom();
		JSONAtomType getType();
		virtual void store(char *str) = 0;
		virtual size_t length() = 0;

		void *asNull() const;
		bool asBool() const;
		int asInt() const;
		double asFloat() const;
		const char *asString() const;
		JSONObject *asObject() const;
		JSONObject &asObjectRef() const;
		JSONArray *asArray() const;
		JSONArray &asArrayRef() const;
	};

	class rSON_CLS_API JSONNull : public JSONAtom
	{
	public:
		JSONNull();
		~JSONNull();
		size_t length();
		void store(char *str);
	};

	class rSON_CLS_API JSONFloat : public JSONAtom
	{
		double value;

	public:
		JSONFloat(double floatValue);
		~JSONFloat();
		operator double() const;
		size_t length();
		void store(char *str);
	};

	class rSON_CLS_API JSONInt : public JSONAtom
	{
	private:
		int value;

	public:
		JSONInt(int intValue);
		~JSONInt();
		operator int() const;
		size_t length();
		void store(char *str);
	};

	class rSON_CLS_API JSONString : public JSONAtom
	{
	private:
		char *value;

	public:
		JSONString(char *strValue);
		~JSONString();
		operator const char *() const;
		size_t length();
		void store(char *str);
	};

	class rSON_CLS_API JSONBool : public JSONAtom
	{
	private:
		bool value;

	public:
		JSONBool(bool boolValue);
		~JSONBool();
		operator bool() const;
		size_t length();
		void store(char *str);
	};

	class rSON_CLS_API JSONObject : public JSONAtom
	{
	private:
		typedef std::map<char *, JSONAtom *, StringLess> atomMap;
		typedef atomMap::iterator atomMapIter;
		atomMap children;
		std::vector<const char *> mapKeys;

	public:
		JSONObject();
		~JSONObject();
		void add(char *key, JSONAtom *value);
		JSONAtom *operator [](const char *key);
		std::vector<const char *> &keys();
		size_t size();
		size_t length();
		void store(char *str);
	};

	class rSON_CLS_API JSONArray : public JSONAtom
	{
	private:
		std::vector<JSONAtom *> children;

	public:
		JSONArray();
		~JSONArray();
		void add(JSONAtom *value);
		JSONAtom *operator [](size_t key);
		size_t size();
		size_t length();
		void store(char *str);
	};

	rSON_API JSONAtom *parseJSON(const char *json);

	rSON_API char *writeJSON(JSONAtom *atom);
	rSON_API void freeString(char **str);
}

#endif /*__rSON_H__*/
