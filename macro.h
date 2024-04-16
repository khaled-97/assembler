#ifndef MACRO_H
#define MACRO_H

#include <stdio.h>

#define MAX_LINE_LENGTH 81
#define MAX_MACRO_LINES 100
#define MAX_LABEL_LENGTH 32
#define MAX_LABELS 100

typedef struct macroline {
    char line[MAX_LINE_LENGTH];
    struct macroline *next;
} macroline;

typedef struct macro {
    char name[MAX_LINE_LENGTH];
    macroline *lines;
    struct macro *nextmacro;
} macro;

typedef struct label {
    char name[MAX_LABEL_LENGTH];
    int address;
    struct label *next;
} label;

void printErrors(char *, int, Status);
int isDuplicateMacroName(char *, macro *);
int isLabelName(char *, label *);
void extract_labels(char *, label **);
void process_macros(FILE *, FILE *, char *, macro **, label *);
void free_macros(macro *);
void free_macro_lines(macroline *);
void free_labels(label *);



char labels[MAX_LABELS][MAX_LABEL_LENGTH];
int label_count = 0;


#endif
