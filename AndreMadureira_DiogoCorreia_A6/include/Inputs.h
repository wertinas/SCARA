#include <stdio.h>
#include <avr/io.h>
#include "serial_printf.h"


char convNum2Char(uint8_t Number);
uint8_t play(uint8_t *x_init, uint8_t *y_init, uint8_t *x_final, uint8_t *y_final);

unsigned int read_adc(unsigned char chan);
void init_adc(void);
void init_usart(void);

uint8_t convChar2Num(char c);