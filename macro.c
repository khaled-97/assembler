#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "printFunctions.h"

void printErrors(char *filename, int lineCounter, Status status) {
    switch (status) {
        case MacroNameAlreadyExists:
            fprintf(stdout, "%s.as:%d: Macro name already exists.\n", filename, lineCounter);
            break;
        case MacroNameIsLabelName:
            fprintf(stdout, "%s.as:%d: Macro name is a label name.\n", filename, lineCounter);
            break;
        default:
            break;
    }
}

int isDuplicateMacroName(char *name, macro *macros) {
    macro *m;
    for (m = macros; m != NULL; m = m->nextmacro) {
        if (strcmp(name, m->name) == 0) {
            return 1;
        }
    }
    return 0;
}

int isLabelName(char *name, label *labels) {
    label *l;
    for (l = labels; l != NULL; l = l->next) {
        if (strcmp(name, l->name) == 0) {
            return 1;
        }
    }
    return 0;
}


void extract_labels(char *filename, label **labels) {
    char line[81];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\\n", filename);
        return;
    }

    int address = 0;
    while (fgets(line, sizeof(line), file)) {
        address++;
        char *labelName = strtok(line, " ");
        if (labelName != NULL && labelName[strlen(labelName) - 1] == ':') {
            labelName[strlen(labelName) - 1] = '\0';

            label *newLabel = (label *)malloc(sizeof(label));
            strcpy(newLabel->name, labelName);
            newLabel->address = address;
            newLabel->next = *labels;
            *labels = newLabel;
        }
    }

    fclose(file);
}


void process_macros(FILE *inputFile, FILE *outputFile, char *filename, macro **macros, label *labels) {
    char line[MAX_LINE_LENGTH];
    macro *currentMacro = NULL;
    int lineCounter = 0;

    while (fgets(line, MAX_LINE_LENGTH, inputFile)) {
        lineCounter++;
        char tempLine[MAX_LINE_LENGTH];
        strcpy(tempLine, line);

        char *token = strtok(tempLine, " \n");
        if (token == NULL) {
            continue;
        }

        if (currentMacro != NULL) {
            if (strcmp(token, "endmcr") == 0) {
                currentMacro = NULL;
            } else {
                macroline *newLine = (macroline *)malloc(sizeof(macroline));
                strcpy(newLine->line, line);
                newLine->next = currentMacro->lines;
                currentMacro->lines = newLine;
            }
        } else {
            if (strcmp(token, "mcr") == 0) {
                token = strtok(NULL, " \n");
                if (token != NULL) {
                    if (isDuplicateMacroName(token, *macros)) {
                        printErrors(filename, lineCounter, MacroNameAlreadyExists);
                        return;
                    }
                    if (isLabelName(token, labels)) {
                        printErrors(filename, lineCounter, MacroNameIsLabelName);
                        return;
                    }
                    macro *newMacro = (macro *)malloc(sizeof(macro));
                    strcpy(newMacro->name, token);
                    newMacro->lines = NULL;
                    newMacro->nextmacro = *macros;
                    *macros = newMacro;
                    currentMacro = newMacro;
                }
            } else {
                macro *m;
                for (m = *macros; m != NULL; m = m->nextmacro) {
                    if (strcmp(token, m->name) == 0) {
                        macroline *l;
                        for (l = m->lines; l != NULL; l = l->next) {
                            fputs(l->line, outputFile);
                        }
                        break;
                    }
                }
                if (m == NULL) {
                    fputs(line, outputFile);
                }
            }
        }
    }
}


void free_macro_lines(macroline *lines);

void free_macros(macro *macros) {
    macro *m;
    while ((m = macros) != NULL) {
        macros = macros->nextmacro;
        free_macro_lines(m->lines);
        free(m);
    }
}

void free_macro_lines(macroline *lines) {
    macroline *l;
    while ((l = lines) != NULL) {
        lines = lines->next;
        free(l);
    }
}

void free_labels(label *labels) {
    label *l;
    while ((l = labels) != NULL) {
        labels = labels->next;
        free(l);
    }
}




int main() {
    int i;
    FILE *as_file = fopen("ps.as", "r");
    if (as_file == NULL) {
        printf("Could not open file ps.as\n");
        return 1;
    }

    label *labels = NULL;
    extract_labels("ps.as", &labels); /* Extract labels from .as file */
    /* Print labels */
    label *l;
    for (l = labels; l != NULL; l = l->next) {
        printf("%s\n", l->name);
    }

    FILE *am_file = fopen("ps.am", "w");
    if (am_file == NULL) {
        printf("Could not open file ps.am\n");
        fclose(as_file); /* Close as_file before returning */
        return 1;
    }

    macro *macros = NULL;
    process_macros(as_file, am_file, "ps.as", &macros, labels); /* Process macros and output to .am file */
    fclose(as_file); /* Close as_file after it's no longer needed */


    fclose(am_file);
    free_labels(labels); /* Free labels */
    free_macros(macros); /* Free macros */

    return 0;
}

