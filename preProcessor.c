#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 81
#define MAX_MACRO_LINES 100
#define MAX_LABEL_LENGTH 32
#define MAX_LABELS 100

char labels[MAX_LABELS][MAX_LABEL_LENGTH];
int label_count = 0;

void extract_labels(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *label = strtok(line, " ");
        if (label != NULL && label[strlen(label) - 1] == ':') {
            label[strlen(label) - 1] = '\0';
            strcpy(labels[label_count++], label);
        }
    }

    fclose(file);
}

void process_macros(FILE *inputFile, FILE *outputFile) {
    char line[MAX_LINE_LENGTH];
    char macroName[MAX_LINE_LENGTH] = "";
    char macroLines[MAX_MACRO_LINES][MAX_LINE_LENGTH];
    int inMacro = 0;
    int macroLineCount = 0;

    while (fgets(line, MAX_LINE_LENGTH, inputFile)) {
        char tempLine[MAX_LINE_LENGTH];
        strcpy(tempLine, line);

        char *token = strtok(tempLine, " \n");
        if (token == NULL) {
            continue;
        }

        if (inMacro) {
            if (strcmp(token, "endmcr") == 0) {
                inMacro = 0;
            } else {
                strcpy(macroLines[macroLineCount++], line);
            }
        } else {
            if (strcmp(token, "mcr") == 0) {
                token = strtok(NULL, " \n");
                if (token != NULL) {
                    inMacro = 1;
                    macroLineCount = 0;
                    strcpy(macroName, token);
                }
            } else if (strcmp(token, macroName) == 0) {
                for (int i = 0; i < macroLineCount; i++) {
                    fputs(macroLines[i], outputFile);
                }
            } else {
                fputs(line, outputFile);
            }
        }
    }
}


int main() {
    int i;
    FILE *as_file = fopen("ps.as", "r");
    if (as_file == NULL) {
        printf("Could not open file ps.as\n");
        return 1;
    }

    FILE *am_file = fopen("ps.am", "r");
    if (am_file == NULL) {
        printf("Could not open file ps.am\n");
        fclose(as_file); /* Close as_file before returning */
        return 1;
    }
    
    process_macros(as_file, am_file);
    fclose(as_file); /* Close as_file after it's no longer needed */

    extract_labels("ps.am"); /* You should pass the filename to extract_labels */
    for (i = 0; i < label_count; i++) {
    printf("%s\n", labels[i]);
}

    fclose(am_file);

    return 0;
}
