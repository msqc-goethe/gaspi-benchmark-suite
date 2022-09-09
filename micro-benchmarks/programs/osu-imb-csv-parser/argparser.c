/**
* @file argparser.c
* @brief 
* @detail 
* @author Florian Beenen
* @version
*/

#include "argparser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




static char *parseString(char *argvi, char *identifier) {
	char *string = strstr(argvi, identifier);
	if (string != NULL) {
		int argLen = strlen(identifier);
		string += argLen; /* Must succeed. */
		if (*string == '=') { /* We found the right position */
			return string + 1;
		}
	}
	return NULL;
}

/*
*	Funktion interpretiert die angegebenen Parameter
*/
extern argParseStatus_t argparser_parse(int argc, char ** argv, cmdArgs_t *args) {
    char *string = NULL;
	args->processMode = MODE_NONE;
    args->inputFile = ARG_DEFAULT_INPUT_FILE;
    args->outputFile = ARG_DEFAULT_OUTPUT_FILE;
    args->specialReplaceChar = ARG_DEFAULT_SPECIAL_REPLACE_CHAR;
    args->commentChar = ARG_DEFAULT_COMMENT_CHAR;
	args->csvSeparatorChar = ARG_DEFAULT_CSV_SEPARATOR_CHAR;
    args->showHelp = 0;
    {
        int i;
        for (i = 1; i < argc; ++i) {
            {
                int scanres;
                char value;
                scanres = sscanf(argv[i], ARG_IDEN_COMMENT_CHAR"=%c", &value);
                if (scanres == 1) {
                    if (value > 0) {
                        args->commentChar = value;
                    } else {
                        return PARSE_UNKNOWN_ERROR;
                    }
                }
            }
            {
                int scanres;
                char value;
                scanres = sscanf(argv[i], ARG_IDEN_SPECIAL_REPLACE_CHAR"=%c", &value);
                if (scanres == 1) {
                    if (value > 0) {
                        args->specialReplaceChar = value;
                    } else {
                        return PARSE_UNKNOWN_ERROR;
                    }
                }
            }
			{
                int scanres;
                char value;
                scanres = sscanf(argv[i], ARG_IDEN_CSV_SEPARATOR_CHAR"=%c", &value);
                if (scanres == 1) {
                    if (value > 0) {
                        args->csvSeparatorChar = value;
                    } else {
                        return PARSE_UNKNOWN_ERROR;
                    }
                }
            }
			string = parseString(argv[i], ARG_IDEN_INPUT_FILE);
			if (string != NULL) {
				args->inputFile = string;
			}
			string = parseString(argv[i], ARG_IDEN_OUTPUT_FILE);
			if (string != NULL) {
				args->outputFile = string;
			}
			
			
            {
                if (strcmp(ARG_IDEN_MODE_OSU, argv[i]) == 0) {
                    args->processMode = MODE_OSU;
                }
            }
			{
                if (strcmp(ARG_IDEN_MODE_IMB, argv[i]) == 0) {
                    args->processMode = MODE_IMB;
                }
            }
            {
                if (strcmp(ARG_IDEN_HELP, argv[i]) == 0) {
                    args->showHelp = true;
                }
            }
        }
    }
	
	if (!(args->showHelp)) {
		if (args->processMode == MODE_NONE) {
			return PARSE_NO_MODE;
		}
		if (args->inputFile == NULL) {
			return PARSE_NO_INPUT_FILE;
		}
	}
    return PARSE_OK;
}

/*
*	Zeigt eine spezifische Fehlermeldung an
*/
extern void argparser_printErr(argParseStatus_t status) {
    switch (status) {
    case PARSE_OK:
        fprintf(stderr, "Argument parsing successful\n");
        break;
    case PARSE_NO_MODE:
        fprintf(stderr, "No processing mode was specified (either IMB or OSU mode is required)\n");
        break;
    case PARSE_NO_INPUT_FILE:
        fprintf(stderr, "No input file was given.\n");
        break;
    default:
        fprintf(stderr, "Unknown error while parsing arguments.\n");
    }
}

/*
*	Zeigt einen definierten Hilfetext an
*/
extern void argparser_showHelp(char *progName) {
    printf("USAGE for \"%s\"\n", progName);
    printf("###########################\n");
    printf(ARG_IDEN_INPUT_FILE"=VALUE : specify the input file (required)\n");
    printf(ARG_IDEN_OUTPUT_FILE"=VALUE : specify the output file (Default: stdout)\n");
    printf(ARG_IDEN_MODE_OSU" : process the input file in OSU mode.\n");
    printf(ARG_IDEN_MODE_IMB" : process the input file in IMB/GBS mode.\n");
    printf(ARG_IDEN_COMMENT_CHAR"=VALUE : specify the character to express comments in the CSV file with. (Default: Comments are stripped)\n");
    printf(ARG_IDEN_SPECIAL_REPLACE_CHAR"=VALUE : specify the character to replace non A-Za-Z characters in the CSV file with (Default: Strip characters)\n");
    printf(ARG_IDEN_CSV_SEPARATOR_CHAR"=VALUE : specify the character that separates the values in the CSV file (Default: %c)\n", ARG_DEFAULT_CSV_SEPARATOR_CHAR);
    printf(ARG_IDEN_HELP" : displays this help\n");
    printf("###########################\n");
}
