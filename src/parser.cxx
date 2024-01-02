// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-FileCopyrightText: 2012-2014,2016-2018,2020,2023 Rachel Mant <git@dragonmux.network>
// SPDX-FileCopyrightText: 2021 Amyspark <amy@amyspark.me>
// SPDX-FileCopyrightText: 2023 Aki Van Ness <aki@lethalbit.net>
// SPDX-FileContributor: Written by Rachel Mant <git@dragonmux.network>
// SPDX-FileContributor: Modified by Amyspark <amy@amyspark.me>
// SPDX-FileContributor: Modified by Aki Van Ness <aki@lethalbit.net>

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(_MSC_VER) || defined(__MACOS__) || defined(__MACOSX__) || defined(__APPLE__)
#define pow10(x) pow(10.0, (int)x)
#endif
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#define O_NOCTTY O_BINARY
#endif

#include "internal/types.hxx"
#include "internal/parser.hxx"

// Recognise lower-case letters
inline bool isLowerAlpha(const char x) noexcept
{
	return x >= 'a' && x <= 'z';
}

// Recognise ASCII control characters
inline bool isControl(const char x) noexcept
{
	return (x >= 0 && x <= 0x1F) || x == 0x7F;
}

// Recognise characters in the file which are deemed in the JSON alphabet
inline bool isAllowedAlpha(const char x) noexcept
{
	return x != '"' && x != '\\' && isControl(x) == false;
}

// Recognise standard English numbers
inline bool isNumber(const char x) noexcept
{
	return x >= '0' && x <= '9';
}

inline bool isMinus(const char x) noexcept
	{ return x == '-'; }

// Recognise the beginning of an object
inline bool isObjectBegin(const char x) noexcept
{
	return x == '{';
}

// Recognise the end of an object
inline bool isObjectEnd(const char x) noexcept
{
	return x == '}';
}

// Recognise the beginning of an array
inline bool isArrayBegin(const char x) noexcept
{
	return x == '[';
}

// Recognise the end of an array
inline bool isArrayEnd(const char x) noexcept
{
	return x == ']';
}

// Recognise a back-slash
inline bool isSlash(const char x) noexcept
{
	return x == '\\';
}

// Recognise a double-quote
inline bool isQuote(const char x) noexcept
{
	return x == '"';
}

// Recognise an exponent delimiter
inline bool isExponent(const char x) noexcept
{
	return x == 'e' || x == 'E';
}

// Recognise a new line character OS agnoistically
inline bool isNewLine(const char x) noexcept
{
	return x == '\n' || x == '\r';
}

// Recognise whitespace
inline bool isWhiteSpace(const char x) noexcept
{
	return x == ' ' || x == '\t' || isNewLine(x);
}

// Recognise a hexadecimal digit
inline bool isHex(const char x) noexcept
{
	return (x >= '0' && x <= '9') ||
		(x >= 'A' && x <= 'F') ||
		(x >= 'a' && x <= 'f');
}

inline bool isOct(const char x) noexcept
{
	return x >= '0' && x <= '7';
}

inline bool isBin(const char x) noexcept
{
	return x == '0' || x == '1';
}

inline bool isBasePrefix(const char x) noexcept
{
	return x == 'x' || x == 'b' || x == 'o';
}

JSONParser::JSONParser(stream_t &toParse) : json(toParse), next(0), lastWasComma(false)
	{ nextChar(); }

void JSONParser::nextChar()
{
	if (json.atEOF())
		throw JSONParserError(JSON_PARSER_EOF);
	json.read(next);
}

// This function intentionally ignores EOF to prevent the
// parser from exiting via exception when it sees the final } or ]
void JSONParser::skipWhite()
{
	while (!json.atEOF() && isWhiteSpace(currentChar()))
		nextChar();
}

// Match the current character with x, and skip whitespace if skip == true.
// Throws an exception if x and the current character do not match.
void JSONParser::match(const char x, const bool skip)
{
	if (currentChar() == x)
	{
		lastWasComma = x == ',';
		nextChar();
		if (skip)
			skipWhite();
	}
	else
		throw JSONParserError(JSON_PARSER_BAD_JSON);
}

bool JSONParser::lastTokenComma() const noexcept { return lastWasComma; }
void JSONParser::lastNoComma() noexcept { lastWasComma = false; }

char JSONParser::currentChar()
{
	if (json.atEOF())
		throw JSONParserError(JSON_PARSER_EOF);
	return next;
}

inline char pop(std::queue<char> &queue) noexcept
{
	char result = queue.front();
	queue.pop();
	return result;
}

// Parses an alpha-numeric literal
char *JSONParser::literal()
{
	if (!isLowerAlpha(currentChar()))
		throw JSONParserError(JSON_PARSER_BAD_JSON);
	std::queue<char> result;

	while (isLowerAlpha(currentChar()))
	{
		result.push(currentChar());
		nextChar();
	}
	skipWhite();

	return [&](char *const literal) noexcept -> char *
	{
		literal[result.size()] = 0;
		for (size_t i = 0; !result.empty(); ++i)
			literal[i] = pop(result);
		return literal;
	}(new char[result.size() + 1]);
}

// Verifies a \u sequence
void JSONParser::validateUnicodeSequence(std::queue<char> &result)
{
	char len = 0;
	for (len = 0; len < 4; len++)
	{
		result.push(currentChar());
		nextChar();
		if (!isHex(currentChar()))
			break;
	}
	if (len != 4)
		throw JSONParserError(JSON_PARSER_BAD_JSON);
}

// Parses a string per the JSON string rules
std::string JSONParser::string()
{
	match('"', false);
	bool slash = false;
	std::queue<char> result;

	while (!isQuote(currentChar()) || slash)
	{
		if (slash)
		{
			switch (currentChar())
			{
				case '"':
				case '\\':
				case '/':
				case 'b':
				case 'f':
				case 'n':
				case 'r':
				case 't':
					break;
				case 'u':
					validateUnicodeSequence(result);
					// Make sure there are 4 hex characters here
					break;
				default:
					throw JSONParserError(JSON_PARSER_BAD_JSON);
			}
			slash = false;
		}
		else
		{
			slash = isSlash(currentChar());
			if (!slash && !isAllowedAlpha(currentChar()))
				throw JSONParserError(JSON_PARSER_BAD_JSON);
		}
		result.push(currentChar());
		nextChar();
	}

	match('"', true);
	return [&]() noexcept
	{
		std::string string(result.size(), '\0');
		for (size_t i = 0; !result.empty(); ++i)
			string[i] = pop(result);
		return string;
	}();
}

// Parses a positive natural number
size_t JSONParser::number(const bool zeroSpecial, size_t *const decDigits)
{
	uint8_t base{10};
	bool (*isValidDigit)(const char) = isNumber;

	const auto nextDigit = [=]()
	{
		if (decDigits)
			++(*decDigits);
		nextChar();
	};

	if (!isNumber(currentChar()))
		throw JSONParserError(JSON_PARSER_BAD_JSON);

	if (zeroSpecial && currentChar() == '0')
	{
		nextDigit();
		if (!isBasePrefix(currentChar()))
		{
			if (isNumber(currentChar()))
				throw JSONParserError(JSON_PARSER_BAD_JSON);
			return 0;
		}
		else
		{
			if (currentChar() == 'x')
			{
				base = 16;
				isValidDigit = isHex;
			}
			else if (currentChar() == 'o')
			{
				base = 8;
				isValidDigit = isOct;
			}
			else if (currentChar() == 'b')
			{
				base = 2;
				isValidDigit = isBin;
			}

			nextChar();
		}
	}

	size_t num = 0;
	while (isValidDigit(currentChar()))
	{
		auto digit{static_cast<uint8_t>(currentChar()  - '0')};
		if (digit > 9U)
			digit -= 7U;

		num *= base;
		num += digit;
		nextDigit();
	}
	return num;
}

// Parses an object
std::unique_ptr<JSONAtom> object(JSONParser &parser)
{
	auto object{std::make_unique<JSONObject>()};
	parser.match('{', true);
	while (isObjectEnd(parser.currentChar()) == false)
	{
		auto key{parser.string()};
		parser.match(':', true);
		object->add(std::move(key), expression(parser));
	}
	if (parser.lastTokenComma())
		throw JSONParserError(JSON_PARSER_BAD_JSON);
	parser.match('}', true);
	return object;
}

// Parses an array
std::unique_ptr<JSONAtom> array(JSONParser &parser)
{
	auto array{std::make_unique<JSONArray>()};
	parser.match('[', true);
	while (!isArrayEnd(parser.currentChar()))
		array->add(expression(parser));
	if (parser.lastTokenComma())
		throw JSONParserError(JSON_PARSER_BAD_JSON);
	parser.match(']', true);
	return array;
}

// Raise 10 to the power of power.
// This is intentionally limited to the positive natural numbers.
size_t power10(size_t power)
{
	size_t i, ret = 1;
	for (i = 0; i < power; i++)
		ret *= 10;
	return ret;
}

// Parses a JSON number, using the positive natural parser.
std::unique_ptr<JSONAtom> number(JSONParser &parser)
{
	bool sign = false, mulSign = false;
	int64_t integer = 0;
	size_t decimal = 0, decDigits = 0, multiplier = 0;
	bool decimalValid = false;

	if (parser.currentChar() == '-')
	{
		parser.match('-', false);
		sign = true;
	}
	integer = parser.number(true);
	if (parser.currentChar() == '.')
	{
		parser.match('.', false);
		decimal = parser.number(false, &decDigits);
		decimalValid = true;
	}
	if (isExponent(parser.currentChar()))
	{
		parser.nextChar();
		if (parser.currentChar() == '-')
		{
			parser.match('-', false);
			mulSign = true;
		}
		else if (parser.currentChar() == '+')
			parser.match('+', false);
		multiplier = parser.number(true);
	}
	parser.skipWhite();

	if (!decimalValid)
	{
		const int64_t mul = power10(multiplier);
		if (mulSign)
			integer /= mul;
		else
			integer *= mul;
		if (sign)
			return std::make_unique<JSONInt>(-integer);
		return std::make_unique<JSONInt>(integer);
	}
	else
	{
		const int64_t mul = power10(multiplier);
		double num = double(decimal) / power10(decDigits);
		num += integer;
		if (mulSign)
			num /= mul;
		else
			num *= mul;
		if (sign)
			return std::make_unique<JSONFloat>(-num);
		return std::make_unique<JSONFloat>(num);
	}
}

// Parses the literals "true", "false" and "null"
std::unique_ptr<JSONAtom> literal(JSONParser &parser)
{
	std::unique_ptr<char []> lit(parser.literal());
	if (strcmp(lit.get(), "true") == 0)
		return std::make_unique<JSONBool>(true);
	else if (strcmp(lit.get(), "false") == 0)
		return std::make_unique<JSONBool>(false);
	else if (strcmp(lit.get(), "null") == 0)
		return std::make_unique<JSONNull>();
	throw JSONParserError(JSON_PARSER_BAD_JSON);
}

// Parses an expression of some sort
std::unique_ptr<JSONAtom> expression(JSONParser &parser, const bool matchComma)
{
	std::unique_ptr<JSONAtom> atom{};
	switch (parser.currentChar())
	{
		case '{':
			atom = object(parser);
			break;
		case '[':
			atom = array(parser);
			break;
		case '"':
			atom = std::make_unique<JSONString>(parser.string());
			break;
	}

	if (atom == nullptr)
	{
		if (isNumber(parser.currentChar()) || isMinus(parser.currentChar()))
			atom = number(parser);
		else
			atom = literal(parser);
	}

	if (matchComma && !isObjectEnd(parser.currentChar()) && !isArrayEnd(parser.currentChar()))
		parser.match(',', true);
	else
		parser.lastNoComma();
	return atom;
}

// The parser entry point
// This verifies the first character in the string to parse is the beginning of either an array or an object
// It then performs a try-catch in which expression() is invoked. if an exception is thrown or needs to be thrown,
// the parser object this temporarily creates is cleaned up before the exception is (re)thrown.
// If everything went OK, this then cleans up the parser object and returns the resulting JSONAtom tree.
std::unique_ptr<JSONAtom> rSON::parseJSON(stream_t &json) try
{
	JSONParser parser(json);
	if (isObjectBegin(parser.currentChar()) || isArrayBegin(parser.currentChar()))
	{
		auto expr = expression(parser, false);
		json.readSync();
		return expr;
	}
	else
		throw JSONParserError(JSON_PARSER_BAD_JSON);
}
catch (JSONParserError &) { json.readSync(); throw; }

std::unique_ptr<JSONAtom> rSON::parseJSON(const std::string_view json)
{
	memoryStream_t stream{const_cast<char *>(json.data()), json.length()};
	return rSON::parseJSON(stream);
}

std::unique_ptr<JSONAtom> rSON::parseJSON(const std::string &json)
	{ return parseJSON(std::string_view{json}); }

std::unique_ptr<JSONAtom> rSON::parseJSON(const char *json)
{
	memoryStream_t stream{const_cast<char *>(json), length(json)};
	return rSON::parseJSON(stream);
}
