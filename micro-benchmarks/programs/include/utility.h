/**
* @file utility.h
* @brief Header fuer hilfreiche Zusatzfunktionen
* @detail Dieser Header definiert nuetzliche Funktionen fuer
*         diverse Anwendungsbereiche.
* @author Florian Beenen
* @version 2018-10-26
*/

#ifndef __UTILITY_H__
#define __UTILITY_H__
#include <stdlib.h>

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define UNUSED(x) (void)(x)

/**
* Datentyp fuer Wahrheitswerte
*/
typedef unsigned char boolean;

#define safeMalloc(byte) safeAlloc(byte, false)
#define safeCalloc(byte) safeAlloc(byte, true)
#define ceilDiv(num, denom) (( (num) + (denom) - 1) / (denom))
#define ptrToInt(ptr) ((int) ((long)(ptr)))
#define intToPtr(i) ((void *) ((long)(i)))
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

/**
* Funktion, um Speicherallokationen durchzufuehrenm, gefolgt von
* einer Pruefung, ob die Allokation erfolgreich war.
* Falls die Allokation nicht erfolgreich war, wird das Programm
* per exit(1) beendet.
*
* @param byte Anzahl der zu allozierenden Bytes
* @param zero Flag, ob der Speicher mit Nullen initialisiert
*        werden soll.
* @return Zeiger auf den allozierten Speicherbereich.
*/
extern void *safeAlloc(size_t byte, boolean zero);

extern char *copyDynamicString(char *buffer);
extern void checkAlloc(void *var);
extern void stripMultiWhitespace(char *str);
extern void stringReplaceAll(char *str, char find, char repl);
extern char *stringSubstring(char *str, unsigned int start, unsigned int end);
extern char *stringConcat(const char *s1, const char *s2);
extern void trimString(char *str);


#endif
