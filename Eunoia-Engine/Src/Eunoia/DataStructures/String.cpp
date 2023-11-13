#include "String.h"
#include "../Math/GeneralMath.h"

namespace Eunoia {

	String::String() :
		m_Chars(0),
		m_Length(0)
	{}

	String::String(u32 length) :
		m_Length(length)
	{
		m_Chars = (char*)malloc(length + 1);
		m_Chars[m_Length] = 0;
	}

	String::String(const char* string)
	{
		m_Length = 0;
		while (string[m_Length] != 0)
			m_Length++;

		m_Chars = (char*)malloc(m_Length + 1);
		memcpy(m_Chars, string, m_Length);
		m_Chars[m_Length] = 0;
	}

	String::String(const String& copy)
	{
		m_Length = copy.m_Length;
		m_Chars = (char*)malloc(m_Length + 1);
		memcpy(m_Chars, copy.m_Chars, m_Length);
		m_Chars[m_Length] = 0;
	}

	String::~String()
	{
		if(m_Chars)
			free(m_Chars);
	}

	char String::CharAt(u32 index) const
	{
		if (index >= m_Length)
			return 0;

		return m_Chars[index];
	}

	u32 String::Length() const
	{
		return m_Length;
	}

	const char* String::C_Str() const
	{
		return m_Chars;
	}

	String String::SubString(u32 startIndex, u32 endIndex) const
	{
		String newString(endIndex - startIndex + 1);
		memcpy(newString.m_Chars, m_Chars + startIndex, endIndex - startIndex + 1);
		return newString;
	}

	String String::SubString(u32 startIndex) const
	{
		return String(m_Chars + startIndex);
	}

	String String::TrimBeginning() const
	{
		char* newString = m_Chars;
		for (u32 i = 0; i < m_Length; i++)
		{
			if (m_Chars[i] == ' ' || m_Chars[i] == '\t' || m_Chars[i] == '\n' || m_Chars[i] == '\r')
				newString++;
			else
				break;
		}

		return String(newString);
	}

	String String::TrimEnding() const
	{
		u32 length = 0;
		for (s32 i = m_Length - 1; i >= 0; i--)
		{
			if (m_Chars[i] == ' ' || m_Chars[i] == '\t' || m_Chars[i] == '\n' || m_Chars[i] == '\r')
				continue;

			length = i + 1;
			break;
		}
		
		String result(length);
		memcpy(result.m_Chars, m_Chars, length);
		return result;
	}

	String String::Trim() const
	{
		if (m_Length == 0)
			return *this;

		s32 startIndex = 0;
		for (s32 i = 0; i < m_Length; i++)
		{
			if (m_Chars[i] == ' ' || m_Chars[i] == '\t' || m_Chars[i] == '\n' || m_Chars[i] == '\r')
				continue;
				
			startIndex = i;
			break;
		}

		u32 endIndex = 0;
		for (s32 i = m_Length - 1; i >= 0; i--)
		{
			if (m_Chars[i] == ' ' || m_Chars[i] == '\t' || m_Chars[i] == '\n' || m_Chars[i] == '\r')
				continue;

			endIndex = i;
			break;
		}

		u32 newLength = endIndex - startIndex + 1;
		String result(endIndex - startIndex + 1);
		memcpy(result.m_Chars, m_Chars + startIndex, newLength);
		return result;
	}

	s32 String::FindFirstOf(const String& substring, u32 offset) const
	{
		u32 count = 0;
		for (u32 i = offset; i < m_Length; i++)
		{
			if (m_Chars[i] == substring[count])
				count++;
			else
				count = 0;

			if (count == substring.m_Length)
				return i - substring.m_Length + 1;
		}

		return -1;
	}

	s32 String::FindLastOf(const String& substring) const
	{
		u32 count = 0;
		for (s32 i = m_Length - 1; i >= 0; i--)
		{
			if (m_Chars[i] == substring[substring.m_Length - count - 1])
				count++;
			else
				count = 0;

			if (count == substring.m_Length)
				return i;
		}

		return -1;
	}

	b32 String::BeginsWith(const String& string) const
	{
		if (m_Length < string.m_Length)
			return false;

		for (u32 i = 0; i < string.m_Length; i++)
			if (m_Chars[i] != string[i])
				return false;

		return true;
	}

	b32 String::Empty() const
	{
		return m_Length == 0;
	}

	s32 String::ParseInt() const
	{
		s32 value;
		sscanf(m_Chars, "%d", &value);
		return value;
	}

	r32 String::ParseFloat() const
	{
		r32 value;
		sscanf(m_Chars, "%f", &value);
		return value;
	}

	char String::operator[](u32 index) const
	{
		return m_Chars[index];
	}

	char& String::operator[](u32 index)
	{
		return m_Chars[index];
	}

	String& String::operator=(const char* string)
	{
		if (!string)
			return *this;

		if(m_Chars)
			free(m_Chars);

		m_Length = 0;
		while (string[m_Length] != 0)
			m_Length++;

		m_Chars = (char*)malloc(m_Length + 1);
		memcpy(m_Chars, string, m_Length);
		m_Chars[m_Length] = 0;

		return *this;
	}

	String& String::operator=(const String& string)
	{
		if (m_Chars)
			free(m_Chars);

		m_Length = string.m_Length;
		m_Chars = (char*)malloc(m_Length + 1);
		memcpy(m_Chars, string.m_Chars, m_Length);
		m_Chars[m_Length] = 0;

		return *this;
	}

	String& String::operator+=(const String& string)
	{
		if (string.m_Length == 0)
			return *this;

		char* newString = (char*)malloc(m_Length + string.m_Length + 1);
		
		if (m_Length > 0)
		{
			memcpy(newString, m_Chars, m_Length);
			free(m_Chars);
		}

		m_Chars = newString;
		for (u32 i = 0; i < string.m_Length; i++)
			m_Chars[m_Length + i] = string[i];

		m_Length += string.m_Length;
		m_Chars[m_Length] = 0;

		return *this;
	}

	b32 String::operator==(const String& string) const
	{
		if (m_Length != string.m_Length)
			return false;

		for (u32 i = 0; i < m_Length; i++)
			if (m_Chars[i] != string[i])
				return false;

		return true;
	}

	b32 String::operator!=(const String& string) const
	{
		return !operator==(string);
	}

	b32 String::operator<(const String& string) const
	{
		return m_Chars[0] < string[0];
	}

	String String::S32ToString(s32 integer)
	{
		u32 length = snprintf(0, 0, "%d", integer);
		String result(length);
		snprintf(result.m_Chars, length + 1, "%d", integer);
		return result;
	}

	String String::R32ToString(r32 real)
	{
		u32 length = snprintf(0, 0, "%f", real);
		String result(length);
		snprintf(result.m_Chars, length + 1, "%f", real);
		return result;
	}

	EU_API String operator+(const String& lhs, const String& rhs)
	{
		String res(lhs.m_Length + rhs.m_Length);
		memcpy(res.m_Chars, lhs.m_Chars, lhs.m_Length);
		memcpy(res.m_Chars + lhs.m_Length, rhs.m_Chars, rhs.m_Length);
		return res;
	}

	EU_API std::ostream& operator<<(std::ostream& stream, const String& string)
	{
		stream << string.m_Chars;
		return stream;
	}

}
