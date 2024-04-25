#include "labelTableLinkedList.h"
#include "firstPass.h"
#include "secondPass.h"
#include "debug.h"
#include "macro.h"
#include <string.h>

void freeVars(variables*);
void addSymbol(SymbolTableEntry**, const char*, int, int, Bool);
void printSymbolTable(SymbolTableEntry *);
Bool isOpcode(char *);
Bool isRegister(char *);
Bool isDefinedVariable(char *, SymbolTableEntry *);
Bool isNumber(char *);
Bool isIndex(char *, label *, SymbolTableEntry *);
Bool isLabel(const char *, label *);
void incrementLineCounter(variables *, char *, char *, SymbolTableEntry *, label *);
void parseLine(char *, char *, char *, label *);
char *trim(char *);


int main(int argc, char *argv[]) {
    char* label;
    int i;
    SymbolTableEntry *symbolTable = NULL;

    process_file(argv[1]);
    
    for(i=1;i<argc;i++) {
        Statement statement;
        variables *variablesPtr;
        char filename[FILE_NAME_LEN + AS_OB_EXTENSION_LEN];
        variablesPtr = (variables*) malloc(sizeof(variables));
        sprintf(filename, "%s.as", argv[i]);
        strcpy(variablesPtr->filename, argv[i]);
        variablesPtr->file = fopen(filename, "r");
        if (!variablesPtr->file) {
            printf("%s: Cannot open file!\n", filename);
            continue;
        }
        fseek(variablesPtr->file, 0, SEEK_END);
        if(ftell(variablesPtr->file) == 0)
            continue;
        rewind(variablesPtr->file);

        variablesPtr->IC = 100;
        variablesPtr->DC = 0;
        variablesPtr->foundError = False;
        variablesPtr->lineCounter = 100;
        variablesPtr->codeHptr = NULL;
        variablesPtr->dataHptr = NULL;
        variablesPtr->labelHptr = NULL;

        while(!feof(variablesPtr->file)) {
            char macro_name[MAX_MACRO_NAME_LENGTH];
            statement = getLine(variablesPtr);

            char operand1[32], operand2[32];

            printf("line is %s\n", variablesPtr->line);
            printf("lineCounter is %d\n", variablesPtr->lineCounter);

            /* Parse the line to extract the operands */
            parseLine(variablesPtr->line, operand1, operand2, variablesPtr->labelHptr);
            /* Check the type of operands and increment the line counter accordingly */
            incrementLineCounter(variablesPtr, operand1, operand2, symbolTable, variablesPtr->labelHptr);
            if(statement == Comment) {
                continue;
            }
            if (strncmp(variablesPtr->line, ".define", 7) == 0) {
                char var[256];
                int val;
                sscanf(variablesPtr->line, ".define %s = %d", var, &val);
                addSymbol(&symbolTable, var, val, 0, False);  /* Add defined variable to symbol table */
            } else {
                label = findLabel(variablesPtr -> line);
                if(label != NULL && label[0] != '\0') {
                    addSymbol(&symbolTable, label, 0, variablesPtr->lineCounter, True);  /* Add label to symbol table */
                }
                if (variablesPtr->line[0] != '.') {
                    variablesPtr->lineCounter++;
                }
            }
        }
        freeVars(variablesPtr);
    }
    printSymbolTable(symbolTable);
    return 0;
}



void freeVars(variables *variablesPtr) {
    freeLabelList(&variablesPtr->labelHptr);
    freeList(&variablesPtr->codeHptr);
    freeList(&variablesPtr->dataHptr);
    free(variablesPtr);
}

void defaultValues(variables *variablesPtr) {
    variablesPtr->status = Valid;
    strcpy(variablesPtr->label,"");
}

void addSymbol(SymbolTableEntry **symbolTable, const char *symbol, int value, int lineNumber, Bool isLabel) {
    SymbolTableEntry *newEntry = (SymbolTableEntry *)malloc(sizeof(SymbolTableEntry));
    strcpy(newEntry->symbol, symbol);
    newEntry->value = value;
    newEntry->lineNumber = lineNumber;
    newEntry->isLabel = isLabel;  /* Set the isLabel field */
    newEntry->next = *symbolTable;
    *symbolTable = newEntry;
}


void printSymbolTable(SymbolTableEntry *symbolTable) {
    SymbolTableEntry *entry = symbolTable;
    while (entry != NULL) {
        if (entry->isLabel) { /* If it's a label, print the line number */
            printf("%s\t%d\n", entry->symbol, entry->lineNumber);
        } else { /* If it's a defined variable, print the value */
            printf("%s\t%d\n", entry->symbol, entry->value);
        }
        entry = entry->next;
    }
}


Bool isOpcode(char *opCode) {
    if (strcmp(opCode, "mov") == 0 || strcmp(opCode, "cmp") == 0 || strcmp(opCode, "add") == 0 || strcmp(opCode, "sub") == 0 ||
     strcmp(opCode, "lea") == 0 || strcmp(opCode, "clr") == 0 || strcmp(opCode, "not") == 0 || strcmp(opCode, "inc") == 0 ||
      strcmp(opCode, "dec") == 0 || strcmp(opCode, "jmp") == 0 || strcmp(opCode, "bne") == 0 || strcmp(opCode, "red") == 0 ||
       strcmp(opCode, "prn") == 0 || strcmp(opCode, "jsr") == 0 || strcmp(opCode, "rts") == 0 || strcmp(opCode, "stop") == 0) {
        return True;
    }
    return False;
}

/* Check if the operand is a register */
Bool isRegister(char *operand) {
    if (operand[0] == 'r' && operand[1] >= '0' && operand[1] <= '7' && operand[2] == '\0') {
        return True;
    }
    return False;
}

/* Check if the operand is a defined variable */
Bool isDefinedVariable(char *operand, SymbolTableEntry *symbolTable) {
    SymbolTableEntry *entry = symbolTable;
    printf("Checking if '%s' is a defined variable\n", operand);
    while (entry != NULL) {
        if (strcmp(entry->symbol, operand) == 0 && !entry->isLabel) {
            return True;
        }
        entry = entry->next;
    }
    return False;
}

/* Check if the operand is a number */
Bool isNumber(char *operand) {
    printf("Checking if '%s' is a number\n", operand);
    if (operand[0] == '#') {
        return True;
    }
    return False;
}

/* Check if the operand is an index */
Bool isIndex(char *operand, label *labels, SymbolTableEntry *symbolTable) {
    printf("Checking if '%s' is an index\n", operand);
    char *bracket = strchr(operand, '[');
    if (bracket != NULL && operand[strlen(operand) - 1] == ']') {
        /* Extract the name and the index inside the brackets */
        char name[MAX_LABEL_LENGTH];
        char index[MAX_LABEL_LENGTH];
        strncpy(name, operand, bracket - operand);
        name[bracket - operand] = '\0';
        strncpy(index, bracket + 1, strlen(operand) - (bracket - operand) - 2);
        index[strlen(operand) - (bracket - operand) - 2] = '\0';

        /* Check if the name is a valid label */
        if (isLabel(name, labels)) {
            /* Check if the index is a number or a defined variable */
            if (isNumber(index) || isDefinedVariable(index, symbolTable)) {
                return True;
            }
        }
    }
    return False;
}





Bool isLabel(const char *str, label *labels) {
    printf("Checking if '%s' is a label\n", str);
    label *l;
    for (l = labels; l != NULL; l = l->next) {
        if (strcmp(str, l->name) == 0) {
            return True;
        }
    }
    return False;
}



/* Increment the line counter based on the type of operands */
void incrementLineCounter(variables *variablesPtr, char *operand1, char *operand2, SymbolTableEntry *symbolTable, label *labels) {
    operand1 = trim(operand1);
    operand2 = trim(operand2);
    printf("Incrementing line counter for '%s' and '%s'\n", operand1, operand2);
    printf("Line counter is %d\n", variablesPtr->lineCounter);

    /* Ignore comments and .define lines */
    if (variablesPtr->line[0] == '.' || variablesPtr->line[0] == ';') {
        return;
    }

    /* If it's an opcode with two register operands */
    if (isRegister(operand1) && isRegister(operand2)) {
        variablesPtr->lineCounter += 2;  /* +1 for the line itself and +1 for the two register operands */
    } 
    /* If the operand is a register, a number, a defined variable, or a label */
    else if (isRegister(operand1) || isNumber(operand1) || isDefinedVariable(operand1, symbolTable) || isLabel(operand1, labels)) {
        variablesPtr->lineCounter += 2;  /* +1 for the line itself and +1 for the operand */
    } 
    /* If the operand is an index */
    else if (isIndex(operand1, labels, symbolTable)) {
        variablesPtr->lineCounter += 3;  /* +1 for the line itself and +2 for the index operand */
    }

    /* If there's a second operand */
    if (operand2[0] != '\0') {
        /* If the second operand is a register, a number, a defined variable, or a label */
        if (isRegister(operand2) || isNumber(operand2) || isDefinedVariable(operand2, symbolTable) || isLabel(operand2, labels)) {
            variablesPtr->lineCounter++;  /* +1 for the operand */
        } 
        /* If the second operand is an index */
        else if (isIndex(operand2, labels, symbolTable)) {
            variablesPtr->lineCounter += 2;  /* +2 for the index operand */
        }
    }
}





void parseLine(char *line, char *operand1, char *operand2, label *labels) {
    printf("Parsing line: '%s'\n", line);
    printf("Operand1: '%s'\n", operand1);
    printf("Operand2: '%s'\n", operand2);
    char *token;
    char *lineCopy = malloc(strlen(line) + 1);
    if (lineCopy == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(lineCopy, line);

    token = strtok(lineCopy, " \t\n");
    if (token == NULL || token[0] == ';' || token[0] == '.') {
        operand1[0] = '\0';
        operand2[0] = '\0';
        free(lineCopy);
        return;
    }

    if (isLabel(token, labels)) {  /* Pass labels to isLabel */
        /* Get the next token */
        token = strtok(NULL, " \t\n");
    }

    if (isOpcode(token)) {
        /* Get the next token */
        token = strtok(NULL, ",");
        if (token != NULL) {
            /* Copy the first operand into operand1 */
            strncpy(operand1, token, strlen(token));
            operand1[strlen(token)] = '\0';
            operand1 = trim(operand1); 

            /* Get the next token */
            token = strtok(NULL, "\n");
            if (token != NULL) {
                /* Copy the second operand into operand2 */
                strncpy(operand2, token, strlen(token));
                operand2[strlen(token)] = '\0';
                operand2 = trim(operand2); 
            } else {
                operand2[0] = '\0';
            }
        }
    }
    
    free(lineCopy);
}



char *trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) 
        return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
    return str;
}

/* Parse the line to extract the operands */
/* void parseLine(char *line, char *operand1, char *operand2) {
    char *token;
    char *lineCopy = (char *)malloc(strlen(line) + 1);
    strcpy(lineCopy, line);
    token = strtok(lineCopy, " ");
    if (token == NULL) {
        operand1[0] = '\0';
        operand2[0] = '\0';
        return;
    }
    strcpy(operand1, token);
    token = strtok(NULL, " ");
    if (token == NULL) {
        operand2[0] = '\0';
        return;
    }
    strcpy(operand2, token);
    operand2 = trim(operand2);
    free(lineCopy);
} */
