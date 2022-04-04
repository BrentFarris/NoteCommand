/**
 * @file string.h
 * @author Brent Farris
 * @date September 15, 2020
 * @brief
 */

#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef STR_NO_16
#include <uchar.h>
#include <wchar.h>
#endif	// STR_NO_16

#define STR_EMPTY			"\0"
#define STRINGIFY(x)		#x
#define TOSTRING(x)			STRINGIFY(x)

//size_t strlen(const char* s);

#ifndef STR_NO_16
size_t strlen16(const char16_t* str);

/******************************************************************************\
* Used to convert a wchar_t string to a UTF-8 string (char*)
* Parameter: str The string to be converted into UTF-8
* Parameter: outStr The initialized memory output UTF-8 string
\******************************************************************************/
void wchartou8(const wchar_t* str, char** outStr);

/******************************************************************************\
* Used to convert a UTF-8 string to a wchar_t string
* Parameter: str The string to be converted into wchar_t
* Parameter: outStr The initialized memory output wchar_t string
\******************************************************************************/
void u8towchar(const char* str, wchar_t** outStr);

/******************************************************************************\
* Used to convert a char16_t string to a UTF-8 string (char*)
* Parameter: str The string to be converted into UTF-8
* Parameter: outStr The initialized memory output UTF-8 string
\******************************************************************************/
void str16tou8(const char16_t* str, char** outStr);

/******************************************************************************\
* Used to convert a UTF-8 string to a char16_t string
* Parameter: str The string to be converted into char16_t
* Parameter: outStr The initialized memory output char16_t string
\******************************************************************************/
void u8tostr16(const char* str, char16_t** outStr);
#endif	// STR_NO_16

/**
 * Count the number of characters that make up a UTF-8 string
 * @param[in] str The UTF-8 string buffer
 * @return The length of the string
*/
size_t utf8len(const char* const str);

/**
 * Safely count the number of characters that make up a UTF-8 string
 * @param[in] str The UTF-8 string buffer
 * @param[in] maxLen The maximum length of the input buffer
 * @return The length of the string
*/
size_t utf8len_s(const char* const str, size_t maxLen);

/**
 * Determine if the supplied string is a valid UTF-8 string
 * @param[in] str The UTF-8 string buffer
 * @return False if the string is invalid, otherwise true
*/
bool utf8valid(const char* const str);

/**
 * Safely determine if the supplied string is a valid UTF-8 string
 * @param[in] str The UTF-8 string buffer
 * @param[in] maxLen The maximum length of the input buffer
 * @return False if the string is invalid, otherwise true
*/
bool utf8valid_s(const char* const str, size_t maxLen);

/******************************************************************************\
* Modifies the supplied string to trim the beginning and by removing any
* occurrence of \n, \t, \r, or spaces using the pre-existing memory space
* Parameter: str The string to be modified by trimming "white space"
\******************************************************************************/
void trim(char* str);

/******************************************************************************\
* Using a provided string, copy a section of that string into a new string
* Parameter: str The string to copy a section from
* Parameter: start The left offset to start copying from
* Parameter: length The number of bytes to copy from the source string
* Parameter: outStr The initialized memory output string
\******************************************************************************/
void substr(const char* str, int32_t start, int32_t len, char** outStr);

/******************************************************************************\
* Join 2 strings together using the "glue" as the string that separates them
* into a single string
* Parameter: lhs The left side of the string
* Parameter: rhs The right side of the string
* Parameter: glue The string to be copied in-between the lhs and rhs strings
* Parameter: outStr The initialized memory output string
\******************************************************************************/
void strjoin(const char* lhs,
	const char* rhs, const char* glue, char** outStr);

/******************************************************************************\
* Allocates enough memory into address pointed to by outStr and copies the
* contents of the string into the newly allocated memory.
\******************************************************************************/
void strclone(const char* str, char** outStr);

/******************************************************************************\
* Allocates enough memory into address pointed to by outStr and copies the
* contents of the string into the newly allocated memory. This will also free
* the existing string if needed
\******************************************************************************/
void strcloneclr(const char* str, char** outStr);

void strsub(char* str, char find, char replace);

/******************************************************************************\
* Find the index of a matching sub-string (search) within a supplied string
* Returns:   The index in the string that matches the search or -1 if not found
* Parameter: str The string used for searching
* Parameter: search The string to match within the search
* Parameter: offset The left offset to start searching from (0 for beginning)
\******************************************************************************/
int32_t stridxof(const char* str, const char* search, int32_t offset);
int32_t stridxofi(const char* str, const char* search, int32_t offset);

/******************************************************************************\
* Find the index of a matching sub-string (search) within a supplied string
* Returns:   The index in the string that matches the search or -1 if not found
* Parameter: str The string used for searching
* Parameter: search The string to match within the search
* Parameter: offset The right offset to start searching from (0 for end)
\******************************************************************************/
int32_t stridxoflast(const char* str, const char* search, int32_t offset);

/******************************************************************************\
* Compares the end of the supplied string for a search string. If a match is
* found then 0 will be returned, otherwise -1 or 1 if the supplied string is
* less than or greater than respectively (same as stdlib strcmp function)
* Returns:   0 if matching end, -1 if str is <, otherwise 1 (same as strcmp)
* Parameter: str The string to be searched within
* Parameter: search The string to compare the end of str with
\******************************************************************************/
int strcmpend(const char* str, const char* search);

/******************************************************************************\
* Count the occurrences of the character in the supplied string
* Returns:   The number of times the searched character occurs
* Parameter: str The string to search within
* Parameter: needle The character to search for
\******************************************************************************/
int32_t strcount(const char* str, unsigned char needle);

/******************************************************************************\
* Determines if the supplied string represents a decimal number
* Returns:   True if the string is a decimal representation
* Parameter: str The string to check
\******************************************************************************/
bool strisdec(const char* str);

/******************************************************************************\
* Determines if the supplied string represents a hexadecimal number. The string
* should be prefixed with 0x
* Returns:   True if the string is a hexadecimal representation
* Parameter: str The string to check
\******************************************************************************/
bool strishex(const char* str);

/******************************************************************************\
* Determines if the supplied string represents a binary number. The string
* should be prefixed with 0b
* Returns:   True if the string is a binary representation
* Parameter: str The string to check
\******************************************************************************/
bool strisbin(const char* str);

/******************************************************************************\
* Returns: True if the strings match with ignored casing
\******************************************************************************/
bool streqi(const char* a, const char* b);

int8_t strtoint8(const char* str);
uint8_t strtouint8(const char* str);
int16_t strtoint16(const char* str);
uint16_t strtouint16(const char* str);
int32_t strtoint32(const char* str);
uint32_t strtouint32(const char* str);
int64_t strtoint64(const char* str);
uint64_t strtouint64(const char* str);
void str16_to_utf8(const char16_t* str, char** outStr);
void utf8_to_str16(const char* str, char16_t** outStr);

int32_t strsplit(const char* a, unsigned char delimiter, char*** out);

void strsplice(const char* str, int32_t start, int32_t len, char** outStr);

static inline size_t strsize(const char* str) {
	return str == NULL ? 0 : strlen(str);
}

static inline void strtolower(char* str) {
	for (; *str != '\0'; str++)
		*str = tolower(*str);
}

static inline bool strempty(const char* str) {
	return str == NULL || str[0] == '\0';
}

static inline bool streq(const char* a, const char* b)
{
	return strcmp(a, b) == 0;
}

#endif	// STRING_H