#ifndef __CSV_OSU_PARSER_H__
#define __CSV_OSU_PARSER_H__

#include "utility.h"
#include "argparser.h"
#include "arraylist.h"

boolean csvOsuParser_process(cmdArgs_t *conf, arraylist_t *lines);

#endif