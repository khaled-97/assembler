#include "labelTableLinkedList.h"
#include "firstPass.h"
#include "secondPass.h"
#include "debug.h"

void freeVars(variables*);
void addSymbol(SymbolTableEntry**, const char*, int, int, Bool);
void printSymbolTable(SymbolTableEntry *);
Bool isRegister(char *);
Bool isDefinedVariable(char *, SymbolTableEntry *);
Bool isNumber(char *);
Bool isIndex(char *);
void incrementLineCounter(variables *, char *, char *, SymbolTableEntry *);
void parseLine(char *line, char *operand1, char *operand2);
char *trim(char *);



int main(int argc, char *argv[]) {
    char* label;
    int i;
    SymbolTableEntry *symbolTable = NULL;
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
            char operand1[32], operand2[32];
            statement = getLine(variablesPtr);
            if(statement == Comment) {
                continue;
            }

            /* Parse the line to extract the operands */
            parseLine(variablesPtr->line, operand1, operand2);

            /* Check the type of operands and increment the line counter accordingly */
            incrementLineCounter(variablesPtr, operand1, operand2, symbolTable);
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
    if (operand[0] == '#') {
        return True;
    }
    return False;
}

/* Check if the operand is an index */
Bool isIndex(char *operand) {
    char *bracket = strchr(operand, '[');
    if (bracket != NULL && operand[strlen(operand) - 1] == ']') {
        return True;
    }
    return False;
}

int isLabel(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    if (str[0] == ' ') {
        return 0;
    }
    char *colonPos = strchr(str, ':');
    if (colonPos != NULL && *(colonPos + 1) == '\0') {
        return 1;
    }
    return 0;
}

/* Increment the line counter based on the type of operands */
void incrementLineCounter(variables *variablesPtr, char *operand1, char *operand2, SymbolTableEntry *symbolTable) {
    printf("operand1 is=%s, operand2 is=%s\n", operand1, operand2);
    /* Ignore comments and .define lines */
    if (variablesPtr->line[0] == ';' || strncmp(variablesPtr->line, ".define", 7) == 0) {
        return;
    }
    SymbolTableEntry *entry = symbolTable;

    /* Increment the line counter based on the type of operands */
    if (isRegister(operand1) || isRegister(operand2)) {
        variablesPtr->lineCounter += 1;
    }
    if (isIndex(operand1) || isIndex(operand2)) {
        variablesPtr->lineCounter += 2;
    }
    if (isNumber(operand1) || isNumber(operand2)) {
        variablesPtr->lineCounter += 1;
    }
    if (entry->isLabel == True ){
        printf("%s is a label\n", entry->symbol);
        variablesPtr->lineCounter += 1;
    }
    if (isDefinedVariable(operand1, symbolTable) || isDefinedVariable(operand2, symbolTable)) {
        variablesPtr->lineCounter += 1;
    }
}


void parseLine(char *line, char *operand1, char *operand2) {
    /* Initialize operands */
    operand1[0] = '\0';
    operand2[0] = '\0';

    /* Check if the line contains a label or directive */
    char *colon = strchr(line, ':');
    char *dot = strchr(line, '.');

    /* Parse the line to extract the operands */
    char *token = strtok(line, " ,\t");
    if (token != NULL) {
        if (token == colon || token == dot) {
            /* Skip the label or directive */
            token = strtok(NULL, " ,\t");
        }
        if (token != NULL) {
            strcpy(operand1, trim(token));
            token = strtok(NULL, " ,\t");
            if (token != NULL) {
                strcpy(operand2, trim(token));
            }
        }
    }
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
