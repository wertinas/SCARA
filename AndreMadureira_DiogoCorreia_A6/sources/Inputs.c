#include <stdio.h>
#include <avr/io.h>
#include "serial_printf.h"
#include "Inputs.h"
#include "timer_tools.h"

#ifndef F_CPU
#define F_CPU 16000000ul
#endif
#define BAUD 57600ul
#define UBBR_VAL ((F_CPU/(BAUD<<3))-1)


void init_usart(void) {
    // Definir baudrate
    UBRR0H = (uint8_t)(UBBR_VAL>>8);
    UBRR0L = (uint8_t) UBBR_VAL;
    UCSR0A = (1<<U2X0); // Double speed
    // Definir formato da trama
    UCSR0C = (3<<UCSZ00) // 8 data bits
    | (0<<UPM00) // no parity
    | (0<<USBS0); // 1 stop bit
    // Ativar recetor e emissor
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

char convNum2Char(uint8_t Number){
    switch (Number)
    {
    case 0:
        return 'A';
    break;
    case 1:
        return 'B';
    break;
    case 2:
        return 'C';
    break;
    case 3:
        return 'D';
    break;
    case 4:
        return 'E';
    break;
    case 5:
        return 'F';
    break;
    case 6:
        return 'G';
    break;
    case 7:
        return 'H';
    break;
    default:
        return ' ';
        break;
    }
}

uint8_t convChar2Num(char c){
    switch (c)
    {
    case 'A':
        return 0;
        break;
    case 'B':
        return 1;
        break;
    case 'C':
        return 2;
        break;
    case 'D':
        return 3;
        break;
    case 'E':
        return 4;
        break;
    case 'F':
        return 5;
        break;
    case 'G':
        return 6;
        break;
    case 'H':
        return 7;
        break;
    }
    return 10;
}


void init_adc(void) {
 // Definir Vref=AVcc
 ADMUX = ADMUX | (1<<REFS0);
 // Desativar buffer digital em PC0
 DIDR0 = DIDR0 | (1<<PC0);
 // Pré-divisor em 128 e ativar ADC
 ADCSRA = ADCSRA | (7<<ADPS0)|(1<<ADEN);
}

unsigned int read_adc(unsigned char chan) {
 // escolher o canal...
 ADMUX = (ADMUX & 0xF0) | (chan & 0x0F);
 // iniciar a conversão
 // em modo manual (ADATE=0)
 ADCSRA |= (1<<ADSC);
 // esperar pelo fim da conversão
 while(ADCSRA & (1<<ADSC));
 return ADC;
}


uint8_t play(uint8_t *x_init, uint8_t *y_init, uint8_t *x_final, uint8_t *y_final){
    //printf("ENTROU PLAY\n");
    init_usart();
    init_adc();
    printf("POS_INIT: [A,1] -> POS_FINAL: [A,1] \n");

    //Letras = colunas, numeros  = linhas

    uint8_t end=0;
    uint8_t pos_xi,pos_yi,pos_xf,pos_yf,pos_select;
    uint8_t xi_state = 0, yi_state = 0, xf_state = 0, yf_state = 0, select = 0;


    while(!end){
        pos_xi = read_adc(0);
        pos_yi = read_adc(1);
        pos_xf = read_adc(2);
        pos_yf = read_adc(3);
        pos_select = read_adc(4);

        switch (xi_state)
        {
            case 0:
                if((pos_xi > 200) & !(yi_state | xf_state |yf_state | select)){
                    // printf("pos_xi: %u\n",pos_xi);
                    xi_state =  1;
                }
                break;
            case 1:
                if(pos_xi < 5){
                    (*x_init) ++;
                    xi_state = 0;
                    if(*x_init > 7) *x_init = 0;
                    printf("POS_INIT: [%c,%u] -> POS_FINAL: [%c,%u]\n",convNum2Char(*x_init),*y_init+1,convNum2Char(*x_final),*y_final+1);
                }
                break;
            default:
                break;
        }

        switch (yi_state)
        {
            case 0:
                if((pos_yi > 200) & !(xi_state | xf_state |yf_state | select)){
                    yi_state =  1;
                } 
                break;
            case 1:
                if(pos_yi < 5){
                    (*y_init) ++;
                    yi_state = 0;
                    if(*y_init > 7) *y_init = 0;
                    printf("POS_INIT: [%c,%u] -> POS_FINAL: [%c,%u]\n",convNum2Char(*x_init),*y_init+1,convNum2Char(*x_final),*y_final+1);
                }
                break; 
            default:
                break;
        }

        switch (xf_state)
        {
            case 0:
                if((pos_xf > 200) & !(xi_state | yi_state |yf_state | select)){
                    xf_state =  1;
                }
                break;
            case 1:
                if(pos_xf < 5){
                    (*x_final) ++;
                    xf_state = 0;
                    if(*x_final > 7) *x_final = 0;
                    printf("POS_INIT: [%c,%u] -> POS_FINAL: [%c,%u]\n",convNum2Char(*x_init),*y_init+1,convNum2Char(*x_final),*y_final+1);
                } 
                break;
            default:
                break;
        }

        switch (yf_state)
        {
            case 0:
                if((pos_yf > 200) & !(xi_state | yi_state | xf_state | select)){
                    yf_state =  1;
                }
                break;
            case 1:
                if((pos_yf < 5)){ // se deixar de pressionar o botao antes do tempo que indica o fim da jogada
                    (*y_final) ++;
                    yf_state = 0;
                    if(*y_final > 7) *y_final = 0;
                    printf("POS_INIT: [%c,%u] -> POS_FINAL: [%c,%u]\n",convNum2Char(*x_init),*y_init+1,convNum2Char(*x_final),*y_final+1);
                }
                break;
            default:
                break;
        }

        // selecao final da jogada
        switch(select){
            case 0:
                if((pos_select > 200) & !(xi_state | yi_state | xf_state | yf_state) ){
                    select = 1;
                }
                break;
            case 1:
                if(pos_select < 5){
                    select  = 0;
                    printf("Jogada selecionada:\n");
                    printf("POS_INIT: [%c,%u] -> POS_FINAL: [%c,%u]\n",convNum2Char(*x_init),*y_init+1,convNum2Char(*x_final),*y_final+1);
                     end = 1;
                }
                break;
            default:
                break;
        }

    }
    return end;


}


