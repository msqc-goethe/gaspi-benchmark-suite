/**
* @file argparser.h
* @brief 
* @detail 
* @author Florian Beenen
* @version 
*/

#ifndef __ARGPARSER_H__
#define __ARGPARSER_H__

#include <stdio.h>
#include "utility.h"


#define ARG_IDEN_MODE_OSU 				"-osu"
#define ARG_IDEN_MODE_IMB 				"-imb"
#define ARG_IDEN_COMMENT_CHAR 			"-c"
#define ARG_IDEN_SPECIAL_REPLACE_CHAR 	"-special"
#define ARG_IDEN_CSV_SEPARATOR_CHAR	    "-sep"
#define ARG_IDEN_INPUT_FILE 			"-i"
#define ARG_IDEN_OUTPUT_FILE 			"-o"
#define ARG_IDEN_HELP					"-h"


#define STRIP_CHAR					((char) 0xFF)

#define ARG_DEFAULT_COMMENT_CHAR 			STRIP_CHAR
#define ARG_DEFAULT_SPECIAL_REPLACE_CHAR	STRIP_CHAR
#define ARG_DEFAULT_CSV_SEPARATOR_CHAR		';'
#define ARG_DEFAULT_INPUT_FILE				NULL
#define ARG_DEFAULT_OUTPUT_FILE				NULL


typedef enum argParseStatus_e {
	PARSE_OK = 0,
	PARSE_NO_MODE,
	PARSE_NO_INPUT_FILE,
	PARSE_UNKNOWN_ERROR
} argParseStatus_t;

typedef enum processMode_e {
	MODE_NONE,
	MODE_OSU,
	MODE_IMB
} processMode_t;

/**
* Struktur fuer die Ausgabe der fertig eingelesenen
* Kommandozeilenparameter.
*/
typedef struct cmdArgs_s {
	processMode_t processMode;
	char *inputFile;
	char *outputFile;
	char specialReplaceChar;
	char commentChar;
	char csvSeparatorChar;
	boolean showHelp;
} cmdArgs_t;

/**
* Funktion, welche das Einlesen der Kommandozeilenparameter
* ausfuehrt und die erkannten Parameter in der uebergebenen
* Struktur ablegt.
*
* @param argc Die Anzahl der Kommandozeilenparameter.
* @param argv Die Kommandozeilenparameter.
* @param args Zeiger auf Speicherbereich fuer die verarbeiteten
*        Kommandozeilenparameter. Speicher muss nicht initialisiert
*        sein.
* @return PARSE_OK, falls alles OK ist, sonst ein fehlerspezifischer Code;
*/
extern argParseStatus_t argparser_parse(int argc, char ** argv, cmdArgs_t *args);

/**
* Funktion, um die Statusnachrichten aus 'argparser_parse' in
* in verstaendlicher Form auf stderr auszugeben.
*
* @param status Statuscode der Argumentenverarbeitung.
*/
extern void argparser_printErr(argParseStatus_t status);

/**
* Schreibt die Erklaerung der Kommandozeilenparameter auf die
* Standardausgabe.
*/
extern void argparser_showHelp(char *progName);

#endif
