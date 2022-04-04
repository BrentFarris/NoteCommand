#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "string.h"

#ifndef MIN
#define MIN(a,b) (a)<(b)?(a):(b)
#endif
#ifndef MAX
#define MAX(a,b) (a)>(b)?(a):(b)
#endif

#ifdef TESTING
#include <assert.h>
#endif


#ifndef STR_NO_16
/******************************************************************************/
/******************************************************************************/
/* WCHAR and char16_t functions                                               */
/******************************************************************************/
/******************************************************************************/
size_t strlen16(const char16_t* str) {
	if (str == NULL)
		return 0;
	char16_t* s = (char16_t*)str;
	for (; *s; ++s);
	return s - str;
}

void wchartou8(const wchar_t* str, char** outStr) {
	mbstate_t state;
	memset(&state, 0, sizeof state);
	const wchar_t* wStr = (wchar_t*)str;
	size_t len = 1 + wcsrtombs(NULL, &wStr, 0, &state);
	*outStr = (char*)malloc(len + 1);
	wcsrtombs(*outStr, &wStr, len, &state);
}

void u8towchar(const char* str, wchar_t** outStr) {
	size_t len = mbstowcs(NULL, str, 0);
	wchar_t* wStr = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	mbstowcs(wStr, str, len);
	*(wStr + len) = '\0';
	*outStr = (wchar_t*)wStr;	// void* here to avoid compiler warning
}

void str16tou8(const char16_t* str, char** outStr) {
	wchartou8((wchar_t*)str, outStr);
}

void u8tostr16(const char* str, char16_t** outStr) {
	u8towchar(str, (wchar_t**)outStr);
}
#endif

/******************************************************************************/
/******************************************************************************/
/* UTF-8 functions                                                            */
/******************************************************************************/
/******************************************************************************/
size_t utf8len(const char* const str) {
	size_t len = 0;
	unsigned char c = str[0];
	for (size_t i = 0, inc = 0; c != 0; ++len) {
		inc = 1;
		if ((c & 0x80)) {
			inc = c >> 4;
			inc -= 11;
			// Invalid increment
			if (inc == 0 || inc > 4)
				return 0;
		}
		i += inc;
		c = str[i];
	}
	return len;
}

size_t utf8len_s(const char* const str, size_t maxLen) {
	size_t len = 0;
	unsigned char c = str[0];
	for (size_t i = 0, inc = 0; c != 0 && i < maxLen; ++len) {
		inc = 1;
		if ((c & 0x80)) {
			inc = c >> 4;
			inc -= 11;
			// Invalid increment
			if (inc == 0 || inc > 4)
				return 0;
		}
		i += inc;
		c = str[i];
	}
	return len;
}

bool utf8valid(const char* const str) {
	if (str == NULL)
		return false;
	unsigned char c = str[0];
	for (size_t i = 1, inc = 0; c != 0; ++i) {
		if (inc > 1) {
			if ((c & 0xC0) != 0x80)
				return false;
			inc--;
		}
		else {
			inc = 1;
			if ((c & 0x80)) {
				inc = c >> 4;
				inc -= 11;
				// Invalid string
				if (inc == 0 || inc > 4)
					return false;
			}
		}
		c = str[i];
	}
	return true;
}

bool utf8valid_s(const char* const str, size_t maxLen) {
	if (str == NULL)
		return false;
	unsigned char c = str[0];
	for (size_t i = 1, inc = 0; c != 0; ++i) {
		if (i == maxLen) {
			return false;
		}
		else if (inc > 1) {
			if ((c & 0xC0) != 0x80)
				return false;
			inc--;
		}
		else {
			inc = 1;
			if ((c & 0x80)) {
				inc = c >> 4;
				inc -= 11;
				// Invalid string
				if (inc == 0 || inc > 4)
					return false;
			}
		}
		c = str[i];
	}
	return true;
}

/******************************************************************************/
/******************************************************************************/
/* String modifications                                                       */
/******************************************************************************/
/******************************************************************************/
void trim(char* str) {
	int32_t len = (int32_t)strlen(str);
	int32_t i;
	for (i = 0; i < len; ++i)
		if (!isspace(str[i]))
			break;
	len -= i;
	memmove(str, str + i, len);
	for (i = len - 1; i >= 0; --i)
		if (!isspace(str[i]))
			break;
	str[i + 1] = '\0';
}

void substr(const char* str, int32_t start, int32_t len, char** outStr) {
	*outStr = NULL;
	if (len > 0) {
		start = MAX(0, start);
		const size_t memLen = MIN((int32_t)strlen(str) - start, len);
		*outStr = (char*)malloc(memLen + 1);
		memcpy(*outStr, str + start, memLen);
		(*outStr)[memLen] = '\0';
	}
}

void strjoin(const char* lhs,
	const char* rhs, const char* glue, char** outStr)
{
	const size_t aLen = lhs != NULL ? strlen(lhs) : 0;
	const size_t bLen = rhs != NULL ? strlen(rhs) : 0;
	if (glue == NULL) {
		const size_t len = aLen + bLen + 1;
		char* joined = (char*)malloc(len);
		memcpy(joined, lhs, aLen);
		memcpy(joined + aLen, rhs, bLen);
		joined[len - 1] = '\0';
		*outStr = joined;
	} else {
		const size_t gLen = strlen(glue);
		const size_t len = aLen + bLen + gLen + 1;
		char* joined = (char*)malloc(len);
		memcpy(joined, lhs, aLen);
		memcpy(joined + aLen, glue, gLen);
		memcpy(joined + aLen + gLen, rhs, bLen);
		joined[len - 1] = '\0';
		*outStr = joined;
	}
}

void strclone(const char* str, char** outStr) {
	char* s = (char*)malloc(strlen(str) + 1);
	strcpy(s, str);
	*outStr = s;
}

void strcloneclr(const char* str, char** outStr) {
	char* s = (char*)malloc(strlen(str) + 1);
	strcpy(s, str);
	free(*outStr);
	*outStr = s;
}

void strsub(char* str, char find, char replace) {
	const size_t len = strlen(str);
	for (size_t i = 0; i < len; ++i)
		if (str[i] == find)
			str[i] = replace;
}

/******************************************************************************/
/******************************************************************************/
/* String queries                                                             */
/******************************************************************************/
/******************************************************************************/
int32_t stridxof(const char* str, const char* search, int32_t offset) {
	int32_t nLen = (int32_t)strlen(search);
	int32_t hLen = (int32_t)strlen(str) - nLen;
	for (int32_t h = offset; h <= hLen; ++h)
		for (int32_t n = 0; n < nLen; ++n) {
			if (str[h + n] != search[n])
				break;
			else if (n == nLen - 1)
				return h;
		}
	return -1;
}

int32_t stridxofi(const char* str, const char* search, int32_t offset) {
	int32_t nLen = (int32_t)strlen(search);
	int32_t hLen = (int32_t)strlen(str) - nLen;
	for (int32_t h = offset; h <= hLen; ++h)
		for (int32_t n = 0; n < nLen; ++n) {
			if (tolower(str[h + n]) != tolower(search[n]))
				break;
			else if (n == nLen - 1)
				return h;
		}
	return -1;
}

int32_t stridxoflast(const char* str, const char* search, int32_t offset) {
	int32_t nLen = (int32_t)strlen(search);
	int32_t hLen = (int32_t)strlen(str) - nLen;
	for (int32_t h = hLen - offset; h >= 0; --h)
		for (int32_t n = 0; n < nLen; ++n) {
			if (str[h + n] != search[n])
				break;
			else if (n == nLen - 1)
				return h;
		}
	return -1;
}

int strcmpend(const char* str, const char* search) {
	const size_t l = strlen(str);
	const size_t r = strlen(search);
	if (r > l)
		return strcmp(str, search);
	return strcmp(str + (l - r), search);
}

int32_t strcount(const char* str, unsigned char needle) {
	int32_t count = 0;
	for (size_t i = 0; i < strlen(str); ++i)
		if (str[i] == needle)
			count++;
	return count;
}

bool strisdec(const char* str) {
	const size_t len = strlen(str);
	size_t i = str[0] == '-' ? 1 : 0;
	if ((len == i + 1 && str[i] == '0'))
		return true;
	if (str[i] == '0' && str[i + 1] != '.')
		return false;
	for (int32_t d = 0; i < len; ++i) {
		if (str[i] == '.') {
			if (++d > 1)
				return false;
		}
		else if (!isdigit(str[i]))
			return false;
	}
	return true;
}

bool strishex(const char* str) {
	const size_t len = strlen(str);
	if (len < 3 || str[0] != '0' || tolower(str[1]) != 'x')
		return false;
	for (size_t i = 2; i < len; i++) {
		const char c = tolower(str[i]);
		if (!isdigit(c) && (c < 'a' || c > 'f'))
			return false;
	}
	return true;
}

bool strisbin(const char* str) {
	const size_t len = strlen(str);
	if (len < 3 || str[0] != '0' || tolower(str[1]) != 'b')
		return false;
	for (size_t i = 2; i < len; i++)
		if (str[i] != '0' && str[i] != '1')
			return false;
	return true;
}

bool streqi(const char* a, const char* b) {
	size_t lenA = strlen(a);
	size_t lenB = strlen(b);
	if (lenA != lenB)
		return false;
	for (size_t i = 0; i < lenA; ++i) {
		if (tolower((int)a[i]) != tolower((int)b[i]))
			return false;
	}
	return true;
}

int32_t strsplit(const char* a, unsigned char delimiter, char*** out) {
	*out = NULL;
	if (a == NULL)
		return 0;
	int32_t len = (int32_t)strlen(a);
	if (len == 0)
		return 0;
	int32_t count = strcount(a, delimiter) + 1;
	char** parts = (char**)malloc(sizeof(char*) * count);
	int32_t start = 0;
	int32_t pIdx = 0;
	for (int32_t i = 0; i < len; ++i) {
		if (a[i] == delimiter) {
			substr(a, start, i - start, &(parts[pIdx]));
			start = ++i;
			pIdx++;
		}
	}
	substr(a, start, len - start, &(parts[pIdx]));
	*out = parts;
	return count;
}

void strsplice(const char* str, int32_t start, int32_t len, char** outStr) {
	size_t sLen = strlen(str);
	char* newStr = malloc((sLen - len) + 1);
	*outStr = newStr;
	for (size_t i = 0, idx = 0; i < sLen; ++i, ++idx) {
		if (i == (size_t)start)
			i += len;
		else
			newStr[idx] = str[i];
	}
	newStr[sLen - len] = '\0';
}

int8_t strtoint8(const char* str) {
	int8_t value;
	if (*str == '0' && *(str + 1) != '\0') {
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int8_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int8_t)strtol((const char*)str, NULL, 2);
		else
			value = (int8_t)strtol((const char*)str, NULL, 8);
	} else
		value = atoi((const char*)str);
	return value;
}

uint8_t strtouint8(const char* str) {
	uint8_t value;
	if (*str == '0' && *(str + 1) != '\0') {
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (uint8_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (uint8_t)strtol((const char*)str, NULL, 2);
		else
			value = (uint8_t)strtol((const char*)str, NULL, 8);
	} else
		value = atoi((const char*)str);
	return value;
}

int16_t strtoint16(const char* str) {
	int16_t value;
	if (*str == '0' && *(str + 1) != '\0'){
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int16_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int16_t)strtol((const char*)str, NULL, 2);
		else
			value = (int16_t)strtol((const char*)str, NULL, 8);
	} else
		value = atoi((const char*)str);
	return value;
}

uint16_t strtouint16(const char* str) {
	uint16_t value;
	if (*str == '0' && *(str + 1) != '\0') {
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (uint16_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (uint16_t)strtol((const char*)str, NULL, 2);
		else
			value = (uint16_t)strtol((const char*)str, NULL, 8);
	} else
		value = (uint16_t)atoi((const char*)str);
	return value;
}

int32_t strtoint32(const char* str) {
	int32_t value;
	if (*str == '0' && *(str + 1) != '\0') {
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int32_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int32_t)strtol((const char*)str, NULL, 2);
		else
			value = (int32_t)strtol((const char*)str, NULL, 8);
	} else
		value = (int32_t)atoi((const char*)str);
	return value;
}

uint32_t strtouint32(const char* str) {
	uint32_t value;
	if (*str == '0' && *(str + 1) != '\0') {
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (uint32_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (uint32_t)strtol((const char*)str, NULL, 2);
		else
			value = (uint32_t)strtol((const char*)str, NULL, 8);
	} else
		value = (uint32_t)atoi((const char*)str);
	return value;
}

int64_t strtoint64(const char* str) {
	int64_t value;
	if (*str == '0' && *(str + 1) != '\0') {
		if (*(str + 1) == 'x' || *(str + 1) == 'X')
			value = (int64_t)strtol((const char*)str, NULL, 16);
		else if (*(str + 1) == 'b' || *(str + 1) == 'B')
			value = (int64_t)strtol((const char*)str, NULL, 2);
		else
			value = (int64_t)strtol((const char*)str, NULL, 8);
	} else
		value = (int64_t)strtol((const char*)str, NULL, 10);
	return value;
}

uint64_t strtouint64(const char* str) {
	return strtoull(str, NULL, 10);
}

void str16_to_utf8(const char16_t* str, char** outStr) {
	mbstate_t state;
	memset(&state, 0, sizeof state);
	const wchar_t* wStr = (wchar_t*)str;
	size_t len = 1 + wcsrtombs(NULL, &wStr, 0, &state);
	*outStr = malloc(len + 1);
	wcsrtombs(*outStr, &wStr, len, &state);
}

void utf8_to_str16(const char* str, char16_t** outStr) {
	size_t len = mbstowcs(NULL, str, 0);
	wchar_t* wStr = malloc((len + 1) * sizeof(wchar_t));
	mbstowcs(wStr, str, len);
	*(wStr + len) = '\0';
	*outStr = (char16_t*)wStr;
}