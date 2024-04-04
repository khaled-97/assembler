#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
#include "defaults.h"


void debug_print(const char* message) {
    printf("\033[1;33m[DEBUG]\033[0m: %s\n", message);
}

void printStatement(Statement statement){
    switch(statement) {
        case Invalid:
            printf("Invalid\n");
            break;
        case Empty:
            printf("Empty\n");
            break;
        case Comment:
            printf("Comment\n");
            break;
        case Directive:
            printf("Directive\n");
            break;
        case Instruction:
            printf("Instruction\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
}
#endif 
