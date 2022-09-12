#include "csv-osu-parser.h"
#include <string.h>
#include "csv-generator.h"


/**
* Remove spacing between "Bandwidth (MB/s)" which is
* inconsistent over the OSU benchmarks. We need to get rid
* of this to create proper CSV columns.
*/
static void fixSpacingHeadingUnit(char *heading) {
	int len = strlen(heading);
	int wi = 0;
	int i = 0;
	for (i = 0; i < len; ++i) {
		char c = heading[i];
		char cn = heading[i + 1];
		if ( !((c == ' ') && (cn != ' '))) {
			heading[wi++] = c;
		}
	}
	heading[wi++] = '\0';
}

boolean csvOsuParser_process(cmdArgs_t *conf, arraylist_t *lines) {
	FILE *outputFile = stdout;
	int headingLineIdx = csvgenerator_getHeadingLineIdx(lines);
	char *heading = arraylist_get(lines, headingLineIdx);
	char *headingFixed = copyDynamicString(heading);
	fixSpacingHeadingUnit(headingFixed);
	if (conf->outputFile != NULL) {
		outputFile = fopen(conf->outputFile, "w");
	}
	
	csvgenerator_printComments(outputFile, lines, headingLineIdx, conf->commentChar);
	csvgenerator_printHeading(outputFile, headingFixed, conf->csvSeparatorChar, conf->specialReplaceChar);
	csvgenerator_printValues(outputFile, lines, headingLineIdx, conf->csvSeparatorChar); 
	
	if (outputFile != stdout) {
		fclose(outputFile);
	}
	free(headingFixed);
	return true;
}