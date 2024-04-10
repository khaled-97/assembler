#ifndef MACRO_H
#define MACRO_H

#include <stdio.h>

#define MAX_LINE_LENGTH 81
#define MAX_MACRO_LINES 100
#define MAX_LABEL_LENGTH 32
#define MAX_LABELS 100


void process_macros(FILE *inputFile, FILE *outputFile);
void extract_labels(char *filename);

#endif