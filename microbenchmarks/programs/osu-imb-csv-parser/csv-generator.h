#ifndef __CSV_GENERATOR_H__
#define __CSV_GENERATOR_H__

#include "utility.h"
#include "argparser.h"
#include "arraylist.h"

/*typedef struct csvgenerator_config_e {
	FILE *in;
	FILE *out;
	processMode_t processMode;
	char blankReplaceChar;
	char braceReplaceChar;
	char commentChar;
	char csvSeparatorChar;
} csvgenerator_config_t;*/

boolean csvgenerator_generate(cmdArgs_t *conf);
char csvgenerator_getFirstNonWhitespaceChar(char *str);
int csvgenerator_getHeadingLineIdx(arraylist_t *lines);
void csvgenerator_printComments(FILE *f, arraylist_t *lines, int stopIdx, char commentChar);
void csvgenerator_printHeading(FILE *f, char *heading, char csvSeparatorChar, char specialReplaceChar);
void csvgenerator_printValues(FILE *f, arraylist_t *lines, int headingIdx, char csvSeparatorChar);


#endif