#ifndef uart_TC
#define uart_TC

extern char trouble_code_buff[6];

static void uart_send_tc();
void uart_start(void);

#endif