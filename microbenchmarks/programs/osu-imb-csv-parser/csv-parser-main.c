#include <stdlib.h>
#include <stdio.h>
#include "argparser.h"
#include "csv-generator.h"


int main(int argc, char *argv[]) {
	cmdArgs_t args;
	/*csvgenerator_config_t csvConfig;*/
	argParseStatus_t argparseStatus = argparser_parse(argc, argv, &args);
	/*FILE *inputFile = NULL;
	FILE *outputFile = NULL; */
	boolean csvGenResult = false;
	if ((argparseStatus != PARSE_OK) || args.showHelp) {
		if (argparseStatus != PARSE_OK) {
			argparser_printErr(argparseStatus);
		}
		argparser_showHelp(argv[0]);
		return argparseStatus;
	}
	printf("Mode: %s\n", (args.processMode == MODE_OSU) ? "OSU" : "IMB");
	printf("Input file: %s\n", args.inputFile);
	printf("Output file: %s\n", args.outputFile);
	printf("Comment char is: %c (%d)\n", args.commentChar, args.commentChar);
	printf("Special char is: %c (%d)\n", args.specialReplaceChar, args.specialReplaceChar);
	printf("CSV sep char is: %c (%d)\n", args.csvSeparatorChar, args.csvSeparatorChar);
	
	/*
	inputFile = fopen(args.inputFile, "r");
	if (args.outputFile == NULL) {
		outputFile = stdout;
	} else {
		outputFile = fopen(args.outputFile, "w");
	}
	if (inputFile == NULL || outputFile == NULL) {
		fprintf(stderr, "Error opening input or output file\n");
		return -1;
	} */
	
	csvGenResult = csvgenerator_generate(&args);
	
	printf("Success? %d\n", csvGenResult);
	
	
	
	/*fclose(inputFile);
	fclose(outputFile); */
	return 0;
}