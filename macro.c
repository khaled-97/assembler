#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "printFunctions.h"
#include "preProcessor.h"

void printErrors(Status status) {
    switch (status) {
        case MacroNameAlreadyExists:
            fprintf(stdout, "Macro name already exists.\n");
            break;
        case MacroNameIsLabelName:
            fprintf(stdout, "Macro name cannot be a label.\n");
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


void extract_labels(const char *filename, label **labels) {
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


void process_macros(FILE *inputFile, FILE *outputFile,const char *filename, macro **macros, label *labels) {
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
                newLine->next = NULL;

                if (currentMacro->lines == NULL) {
                    currentMacro->lines = newLine;
                } else {
                    macroline *lastLine = currentMacro->lines;
                    while (lastLine->next != NULL) {
                        lastLine = lastLine->next;
                    }
                    lastLine->next = newLine;
                }
            }
        } else {
            if (strcmp(token, "mcr") == 0) {
                token = strtok(NULL, " \n");
                if (token != NULL) {
                    if (isDuplicateMacroName(token, *macros)) {
                        printErrors(MacroNameAlreadyExists);
                        return;
                    }
                    if (isLabelName(token, labels)) {
                        printErrors(MacroNameIsLabelName);
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




void process_file(const char *filename) {
    FILE *as_file = fopen(filename, "r");
    if (as_file == NULL) {
        printf("Could not open file %s\n", filename);
        return;
    }

    label *labels = NULL;
    extract_labels(filename, &labels);

    label *l;
    for (l = labels; l != NULL; l = l->next) {
        printf("%s\n", l->name);
    }

    char am_filename[256];
    sprintf(am_filename, "%s.am", filename);
    FILE *am_file = fopen(am_filename, "w");
    if (am_file == NULL) {
        printf("Could not open file %s\n", am_filename);
        fclose(as_file);
        return;
    }

    macro *macros = NULL;
    process_macros(as_file, am_file, filename, &macros, labels);
    fclose(as_file);

    fclose(am_file);
    free_labels(labels);
    free_macros(macros);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <ps>\n", argv[0]);
        return 1;
    }

    process_file(argv[1]);

    return 0;
}
