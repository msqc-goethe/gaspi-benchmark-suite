#include "csv-generator.h"
#include "csv-osu-parser.h"
#include "csv-imb-parser.h"
#include <string.h>
#include <assert.h>

static void checkExtendBuffer(char **buffer, unsigned int *bufferLen, unsigned int *bufferCap) {
	if ((*bufferLen + 1) > *bufferCap) {
		*buffer = (char *) realloc(*buffer, sizeof(char) * (*bufferCap + 10));
		checkAlloc(*buffer);
		*bufferCap += 10;
	}
}

static void readLine(FILE *f, char **buffer,  unsigned int *bufferCap) {
	boolean eol = false;
	unsigned int bufferLen = 0;

	while(!feof(f) && !eol) {
		int c = fgetc(f);
		eol = (c == '\n');
		if (!eol && !feof(f)) {
			checkExtendBuffer(buffer, &bufferLen, bufferCap);
			(*buffer)[bufferLen++] = (char) c;
		}
	}
	checkExtendBuffer(buffer, &bufferLen, bufferCap);
	(*buffer)[bufferLen++] = '\0';
}


static arraylist_t *readLines(FILE *f) {
	unsigned int bufferCap = 0;
	char *readBuffer = NULL;
	arraylist_t * lines = arraylist_create();
	while (!feof(f)) {
		readLine(f, &readBuffer, &bufferCap);
		trimString(readBuffer);
		if (strlen(readBuffer) > 0) {
			char *copy = copyDynamicString(readBuffer);
			arraylist_add(lines, copy);
		}
	}
	
	if (readBuffer != NULL) {
		free(readBuffer);
	}
	return lines;
}

char csvgenerator_getFirstNonWhitespaceChar(char *str) {
	int len = strlen(str);
	int i;
	for (i = 0; i < len; ++i) {
		char c = str[i];
		if (c != ' ' && c != '\t' && c > 30) {
			return c;
		}
	}
	return '\0';
}

int csvgenerator_getHeadingLineIdx(arraylist_t *lines) {
	unsigned int i;
	for (i = 0; i < arraylist_getLength(lines); ++i) {
		char *line = arraylist_get(lines, i);
		char c = csvgenerator_getFirstNonWhitespaceChar(line);
		if ((c >= '0') && (c <= '9')) {
			return i - 1;
		}
	}
	return 0;
}

void csvgenerator_printComments(FILE *f, arraylist_t *lines, int stopIdx, char commentChar) {
	int i;
	if (commentChar != STRIP_CHAR) {
		for (i = 0; i < stopIdx; ++i) {
			char *line = arraylist_get(lines, i);
			char *lineCpy = copyDynamicString(line);
			char *commentStart = strchr(lineCpy, '#');
			if ((commentStart != NULL) && ((*commentStart) != '\0')) {
				*commentStart = commentChar;
			}
			fprintf(f, "%s\n", commentStart);
			free(lineCpy);
		}
	}
}


void csvgenerator_printHeading(FILE *f, char *heading, char csvSeparatorChar, char specialReplaceChar) {
	char *lineCpy = copyDynamicString(heading);
	int i = 0;
	int writeIdx = 0;
	int lineLen = 0;
	stripMultiWhitespace(lineCpy);
	lineLen = strlen(lineCpy);
	for (i = 0; i < lineLen; ++i) {
		char c = lineCpy[i];
		if (( ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) )) {
			lineCpy[writeIdx++] = c;
		} else if (c == ' ') {
			lineCpy[writeIdx++] = csvSeparatorChar;
		} else {
			if (specialReplaceChar != STRIP_CHAR) {
				lineCpy[writeIdx++] = specialReplaceChar;
			} 
		}
	}
	lineCpy[writeIdx++] = '\0';
	fprintf(f, "%s\n", lineCpy);
	free(lineCpy);
}

void csvgenerator_printValues(FILE *f, arraylist_t *lines, int headingIdx, char csvSeparatorChar) {
	int i;
	int len = arraylist_getLength(lines);
	for (i = headingIdx + 1; i < len; ++i) {
		char *line = arraylist_get(lines, i);
		char *lineCpy = copyDynamicString(line);
		stripMultiWhitespace(lineCpy);
		stringReplaceAll(lineCpy, ' ', csvSeparatorChar); 
		if (strlen(lineCpy) > 0) {
			fprintf(f, "%s\n", lineCpy);
		}
		free(lineCpy);
	}
}

boolean csvgenerator_generate(cmdArgs_t *conf) {
	FILE *inputFile = fopen(conf->inputFile, "r");
	arraylist_t *lines = NULL;
	boolean result = false;
	
	if (inputFile == NULL) {
		perror("Cannot open input file");
		exit(1);
	}
	lines = readLines(inputFile);
	
	
	/*{
		unsigned int i;
		for (i = 0; i < arraylist_getLength(lines); ++i) {
			char * line = (char*) arraylist_get(lines, i);
			printf("[%d] --> %s\n", i, line);
		}
	}*/
	
	if (conf->processMode == MODE_OSU) {
		result |= csvOsuParser_process(conf, lines);
	} else if (conf->processMode == MODE_IMB) {
		result |= csvImbParser_process(conf, lines);
	}
	
	fclose(inputFile);
	lines = arraylist_deleteFull(lines, arraylist_freeElementDefault);
	return result;
}