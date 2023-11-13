#pragma once

#include "../Common.h"
#include <iostream>

namespace Eunoia {

    EU_REFLECT()
    class EU_API String
    {
        public:
            String();
		    String(u32 length);
		    String(const char* string);
		    String(const String& copy);
		    ~String();

            /*
                Returns the character at a given index. If the index exceeds the length of the string then the null termination character (0) is returned.
            */
            char CharAt(u32 index) const;

            /*
                Returns the number of characters in the string
            */
            u32 Length() const;

            /*
                returns a constant pointer to the characters of the string
            */
            const char* C_Str() const;

            /*
                Returns a substring including the characters in the range of [startIndex, endIndex]
            */
            String SubString(u32 startIndex, u32 endIndex) const;

            /*
                Returns a substring including the characters in the range of [startIndex, m_Length)
           */
            String SubString(u32 startIndex) const;

            /*
                Removes any blank space in the beginning of the string
                Ex)
                    "   \n\tHello\n" -> TrimBeginning -> "Hello\n"
            */
            String TrimBeginning() const;

             /*
                Removes any blank space in the ending of the string
                Ex)
                    "\nHello   \n\t\t" -> TrimEnding -> "\nHello"
            */
            String TrimEnding() const;

            /*
                Removes any blank space in the beginning and the end of the string
                Ex)
                    "\n  \t Hello \n\n" -> Trim -> "Hello"
            */
            String Trim() const;

            /*
                Returns the first index where a substring occures
                If the substring doesn't occur in the string then -1 is returned
            */
            s32 FindFirstOf(const String& substring, u32 offset = 0) const;

            /*
                Returns the last index where a substring occures
                If the substring doesn't occur in the string then -1 is returned
            */
            s32 FindLastOf(const String& substring) const;

            b32 BeginsWith(const String& string) const;

            b32 Empty() const;

            s32 ParseInt() const;
            r32 ParseFloat() const;

            char* GetChars() { return m_Chars; }

            char operator[](u32 index) const;
            char& operator[](u32 index);

            String& operator=(const char* string);
            String& operator=(const String& string);

            String& operator+=(const String& string);

            b32 operator==(const String& string) const;
            b32 operator!=(const String& string) const;

            b32 operator<(const String& string) const;

            EU_API friend String operator+(const String& lhs, const String& rhs);

            EU_API friend std::ostream& operator<<(std::ostream& stream, const String& string);

            static String S32ToString(s32 integer);
            static String R32ToString(r32 real);
        private:
            EU_PROPERTY() char* m_Chars;
            EU_PROPERTY() u32 m_Length;
    };

}