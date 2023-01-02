/**
* @file utility.c
* @brief Implementationen der hilfreichen Zusatzfunktionen
* @detail Hier werden nuetzliche Funktionen fuer
*         diverse Anwendungsbereiche implementiert.
* @author Florian Beenen
* @version 2018-10-26
*/


#include "utility.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

extern void *safeAlloc(size_t bytes, boolean zero) {
    void *data;
    if (zero) {
        data = calloc(1, bytes);
    } else {
        data = malloc(bytes);
    }
    if (data == NULL) {
        perror("Malloc failed. Not enough memory. Terminating now.");
        exit(1);
    }
    return data;
}

extern void checkAlloc(void *var) {
    if (var == NULL) {
        perror("Mem-Alloc failed. Terminating now.");
        exit(1);
    }
}

extern char *copyDynamicString(char *buffer) {
    int strLen = strlen(buffer);
    char *str = malloc(sizeof(char) * (strLen + 1));
    checkAlloc(str);
    memcpy(str, buffer, strLen + 1);
    return str;
}

extern void stripMultiWhitespace(char *str) {
	int len = strlen(str);
	int rIdx = 0;
	int wIdx = 0;
	boolean wsBefore = true;
	for (rIdx = 0; rIdx < len; ++rIdx) {
		char c = str[rIdx];
		boolean ws = ( (c == ' ') || (c == '\t'));
		if (ws && !wsBefore) {
			str[wIdx++] = ' ';
			wsBefore = true;
		} else if (!ws) {
			str[wIdx++] = c;
			wsBefore = false;
		}
	}
	
	/* remove trailing whitespace */
	do {
		str[wIdx] = '\0';
	} while(str[--wIdx] == ' ');	
}

extern void trimString(char *str) {
	#define isWhitespace(x) ( ( ((x) > 0) && ((x) <= 31)) || ( (x) == ' '))
    assert (str != NULL);
    {
        int charSkip = 0;
        boolean trimFront = true;
        char *introPtr = str;
        while (*str != '\0') {
            str[-charSkip] = *str;
            if (trimFront) {
                if (isWhitespace(*str)) {
                    ++charSkip;
                } else {
                    trimFront = false;
                }
            }
            ++str;
        }
        /* Remove dupliacted Chars at the end*/
        while(charSkip > 0) {
            str[-charSkip--] = ' ';
        }

		/* Iterate backwarts through the string. Running pointer
		* must not overtake initial pointer */
        --str;
        while((str >= introPtr) && isWhitespace(*str)) {
            *str = '\0';
            --str;
        }
    }
	#undef isWhitespace
}


extern void stringReplaceAll(char *str, char find, char repl) {
	int len = strlen(str);
	int i;
	for (i = 0; i < len; ++i) {
		if (str[i] == find) {
			str[i] = repl;
		}
	}
}

extern char *stringConcat(const char *s1, const char *s2) {
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	char *result = safeMalloc(sizeof(char) * l1 + l2 + 1);
	strcpy(result, s1);
	strcpy(result + l1, s2);
	result[l1 + l2] = '\0';
	return result;
}

extern char *stringSubstring(char *str, unsigned int start, unsigned int end) {
	unsigned int len = (end - start);
	char *result;
	assert(start <= end);
	result = safeMalloc(sizeof(char) * (len + 1));
	memcpy(result, str + start, len);
	result[len] = '\0';
	return result;
}