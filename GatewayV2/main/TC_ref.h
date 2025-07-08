#ifndef TC
#define TC

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#define tc_size 5

extern SemaphoreHandle_t TC_Recieved_sem;

char * TC_buff_conv_char(uint8_t TC_buff[2]);
void TC_Code_set(char TC_code[tc_size + 2]);
char* TC_Code_Get();
void new_tc_tasks();

#endif