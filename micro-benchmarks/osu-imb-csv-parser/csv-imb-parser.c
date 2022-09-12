#include "csv-imb-parser.h"
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "csv-generator.h"


static int getNextBenchmarkStartLine(arraylist_t *lines, unsigned int from) {
	unsigned int i;
	for (i = from; i < arraylist_getLength(lines); ++i) {
		char *line = arraylist_get(lines, i);
		/* Search for the word "Benchmarking". */
		char *prefix = strstr(line, "Benchmarking");
		if ((prefix != NULL) && (i != from)) {
			return i - 1;
		}
	}
	return -1;
}


static arraylist_t *splitBenchmarks(arraylist_t *lines) {
	arraylist_t * benchmarkList = arraylist_create();
	int i = 0;
	while (i < (int)arraylist_getLength(lines)) {
		int start = getNextBenchmarkStartLine(lines, i);
		int end = getNextBenchmarkStartLine(lines, start + 1);
		arraylist_t *currentBenchLines = arraylist_create();
		boolean inPrefix = true;
		assert (start >= 0);
		if (end < 0) {
			end = arraylist_getLength(lines);
		}
		for (i = start; i < end; ++i) {
			char *line = arraylist_get(lines, i);
			char first = csvgenerator_getFirstNonWhitespaceChar(line);
						
			if (inPrefix || ((first >= '0') && (first <= '9'))) {
				arraylist_add(currentBenchLines, line);
			}
			
			if (inPrefix && (first != '#')) {
				inPrefix = false;
			}
		}
		arraylist_add(benchmarkList, currentBenchLines);
	}
	return benchmarkList;
}

static char *getBenchmarkName(arraylist_t *benchLines) {
	unsigned int i;
	const char *searchword = "Benchmarking";
	const char *nonaggregate = "NON-AGGREGATE";
	const char *aggregate = "AGGREGATE";
	const char *aggSuffix = "_aggregate";
	const char *nonAggSuffix = "_nonaggregate";
	char *result = NULL;
	for (i = 0; (i < arraylist_getLength(benchLines)) && (result == NULL); ++i) {
		char *line = arraylist_get(benchLines, i);
		/* Search for the word "Benchmarking". */
		char *prefix = strstr(line, searchword);
		if (prefix != NULL) {
			result = copyDynamicString((prefix + strlen(searchword) + 1));
		}
	}
	/* Search for Aggregate Comment. Non-Aggregate has precedence */
	if (result != NULL) {
		boolean aggFound = false;
		for (i = 0; (i < arraylist_getLength(benchLines)) && !aggFound; ++i) {
			char *line = arraylist_get(benchLines, i);
			char *prefixNonAgg = strstr(line, nonaggregate);
			char *prefixAgg = strstr(line, aggregate);
			if (prefixNonAgg != NULL) {
				char *combo = stringConcat(result, nonAggSuffix);
				free(result);
				result = combo;
				aggFound = true;
			} else if (prefixAgg != NULL) {
				char *combo = stringConcat(result, aggSuffix);
				free(result);
				result = combo;
				aggFound = true;
			}
		}
	}
	
	return result;
}

static FILE *createOutputFile(char *outputPath, char *benchmarkName) {
	const char *fileSuffix = ".csv";
	FILE *outputFile = NULL;
	char *outputFileName = NULL;
	struct stat path_stat;
	boolean isFile = false;
	boolean pathExists = false;
	int statOk = 0;
	size_t outputFileNameSize = 0;
	if ((outputPath == NULL) || (benchmarkName == NULL)) {
		return NULL;
	}
	statOk = stat(outputPath, &path_stat);
	pathExists = (statOk == 0);
    isFile = S_ISREG(path_stat.st_mode);

	if (!pathExists) {
		int notOk = mkdir(outputPath, 0755);
		if (notOk) {
			perror("Error creating output directory\n");
			exit(1);
		}
	} else if (pathExists && isFile) {
		fprintf(stderr, "Output path exists and is a file. Must be a directory\n");
		exit(1);
	}
	
	/* Build output file name as: PATH / BENCH-NAME.csv */
	outputFileNameSize = (strlen(outputPath) + strlen(benchmarkName) + strlen("/") + strlen(fileSuffix));
	outputFileName = safeCalloc(sizeof(char) * (outputFileNameSize + 1));
	strcpy(outputFileName, outputPath);
	outputFileName[strlen(outputPath)] = '/';
	strcpy(outputFileName + strlen(outputPath) + strlen("/"), benchmarkName);
	strcpy(outputFileName + strlen(outputPath) + strlen("/") + strlen(benchmarkName), fileSuffix);
	outputFileName[outputFileNameSize] = '\0';
	
	outputFile = fopen(outputFileName, "w");
	free(outputFileName);
	return outputFile;
}

static void benchlistFreeFunc(void *o) {
	if (o != NULL) {
		arraylist_t *l = (arraylist_t *)o;
		arraylist_delete(l);
	}
}



boolean csvImbParser_process(cmdArgs_t *conf, arraylist_t *lines) {
	arraylist_t *benchList = splitBenchmarks(lines);
	unsigned int i;
	for (i = 0; i < arraylist_getLength(benchList); ++i) {
		arraylist_t *bench = arraylist_get(benchList, i);
		char *benchName = getBenchmarkName(bench);
		FILE *outputFile = createOutputFile(conf->outputFile, benchName);
		if (outputFile != NULL) {
			int headingIdx = csvgenerator_getHeadingLineIdx(bench); 
			char *heading = arraylist_get(bench, headingIdx);
			csvgenerator_printComments(outputFile, bench, headingIdx, conf->commentChar);
			csvgenerator_printHeading(outputFile, heading, conf->csvSeparatorChar, conf->specialReplaceChar);
			csvgenerator_printValues(outputFile, bench, headingIdx, conf->csvSeparatorChar); 
			fclose(outputFile);
		} else {
			fprintf(stderr, "Output file cannot be created\n");
			return false;
		}
		if (benchName != NULL) {
			free(benchName);
		}
	}
	
	/*
	{
		unsigned int i;
		for (i = 0; i < arraylist_getLength(benchList); ++i) {
			unsigned int j;
			arraylist_t *bench = arraylist_get(benchList, i);
			for (j = 0; j < arraylist_getLength(bench); ++j) {
				printf("[%d][%d] --> %s\n", i, j, (char *) arraylist_get(bench, j));
			}
			
		}
	} */
	benchList = arraylist_deleteFull(benchList, benchlistFreeFunc);
	return true;
}