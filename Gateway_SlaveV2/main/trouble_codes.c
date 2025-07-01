#include "inttypes.h"
#include "tCodes_list.c"
#include <stdio.h>

typedef struct {
    code_num_t num; //numbers in decimal form example: P34C8 is letter = P and num = code13512
    char letter;    //---------------------------------------------------------------^from fCodes enum
}t_code_t;

void print_code(t_code_t fault_code){
    printf("%c%04d\n",fault_code.letter,fault_code.num);
    fflush(stdout);
}

