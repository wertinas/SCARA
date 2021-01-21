#include <avr\io.h>
#include <avr\interrupt.h>
#include "Move_Servo.h"
#include "serial_printf.h"
//#include <float.h>
#include <stdio.h>
#include <util/delay.h>


uint8_t posicoes[32][2]=  {
                              {0,1},	{0,3},	{0,5},	{0,7},
                            {1,0},	{1,2},	{1,4},	{1,6},
                              {2,1},	{2,3},	{2,5},	{2,7},
                            {3,0},	{3,2},	{3,4},	{3,6},
                              {4,1},	{4,3},	{4,5},	{4,7},
                            {5,0},	{5,2},	{5,4},	{5,6},
                              {6,1},	{6,3},	{6,5},	{6,7},
                            {7,0},	{7,2},	{7,4},	{7,6}
					              	};

Coordenadas peca;
movimentos servo;
uint8_t i, j;
float grid_size, constante;
//grid = quadrícula do jogo

/*Valores Limite: 30-150*/
#define ON 1
#define OFF 0

void servo_tc0_PWM_init(void){
  TCCR0B = 0;   //desliga o timer

  TCCR0B = (1 << WGM02);

  TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1) | (1 << COM0B1);
  //COM0A[1:0] = 1 0    controla OC0A
  //COM0B[1:0] = 1 0    controla OC0B
  //WGM0[2:0] = 1 1 1

  TCNT0 = 0;

  // OCR0A  =  125; // Valor do CNT
  OCR0A = 130;
  //Para fint = 500Hz, TP = 256 e CNT = 125
  
  //OCR0A = 9;
  //OCR0B = 9;

  TCCR0B |= (1<<CS02) | (0<<CS01) | (0<<CS00);

}

void servo_tc1_PWM_init(void) {

  //DDRB = 0b00000110; // coloca o pinb 9 e 10 como saidas

  TCCR1B = 0; // desliga o timer

  TCCR1B = (1<<WGM13) | (1<<WGM12);

  TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (0<<COM1A0) | (0<<COM1B0) | (1<<WGM11);// modo fast PWM e clear on compare match

  TCNT1 = 0;  // Reset do timer

  ICR1 = 40000;  // valor do CNT calculado para freq de 50hz e TP = 8 

  // OCR1A = 3000; //Servo começa no meio (posicao 0) 1.5 ms pulse [Servo base]

  // OCR1B = 3000; //Servo começa no meio  (posicao 0) 1.5ms pulse [Servo topo]

  //posicoes de calibracao
  OCR1A = 3711; // OCR calculado apartir de um angulo de 122 graus
  OCR1B = 3111; // OCR calculado apartir de um angulo de 95 graus

  TCCR1B |= (0 << CS12) | (1<<CS11) | (0<<CS10);

}

void io_init(void){
    DDRB |= (1 << PB1) | (1 << PB2); //coloca o pinb 9 e 10 como saidas (PWM)
    // PINB9 servo Base
    // PINB10 servo topo
    DDRD |= (1 << PD5); //  Servo espeta

    DDRD |= (1 << PD2); //  Eletroiman

    DDRB |= (1 << PB0); //  LED Player 1

    DDRD |= (1 << PD7); //  LED Player 2

}

void move_servo_base(uint8_t angle){
  // angle deve estar entre 0  e 180
  cli();
  OCR1A = (200/9) * angle + 1000;
  sei();
}
void move_servo_topo(uint8_t angle){
  // angle deve estar entre 0  e 180
  cli();
  OCR1B = (200/9) * angle + 1000;
  sei();
}
void move_servo_ponta(uint8_t baixo_cima){
  // Baixo = 1;
  // Cima = 0
  cli();
  // OCR0B = (baixo_cima != 0)? 124:22;//124:22;
  OCR0B = (baixo_cima != 0)? 123:22;

  sei();
}
void iman (uint8_t state) {
  if(state) PORTD |=(1<<PD2);
  else PORTD &= (~(1<<PD2));
}
void calculo_pos(void){
  printf_init();
  //Função vertical por definir
  // Função horizontal por definir 
  // constante = 1.75;  /////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  // Para cada caso fazer relação da rotação servo-base e servo-topo
  // com distância ao ponto 1-8, que corresponde aos servos estarem 90(base)
  // e 140 (topo)
  //Percorre 2cm para cima se for retirado 10 ao servo-base e somado 7 ao servo-topo
  //Percorre 2.5cm para esquerda se for somado 3 ao servo-base e subtraído 10 ao servo-topo
  //Fórmula apresentada para cálculo da posição pode não ser esta  

  servo.movimento_base[0][1]=83;
  servo.movimento_topo[0][1]=119;

  servo.movimento_base[0][3]=101;
  servo.movimento_topo[0][3]=102;

  servo.movimento_base[0][5]=114;
  servo.movimento_topo[0][5]=96;

  servo.movimento_base[0][7]=122;
  servo.movimento_topo[0][7]=95;

  servo.movimento_base[1][0]=89;
  servo.movimento_topo[1][0]=100;

  servo.movimento_base[1][2]=102;
  servo.movimento_topo[1][2]=89;

  servo.movimento_base[1][4]=115;
  servo.movimento_topo[1][4]=81;

  servo.movimento_base[1][6]=125;
  servo.movimento_topo[1][6]=80;
  
  servo.movimento_base[2][1]=99;
  servo.movimento_topo[2][1]=79;

  servo.movimento_base[2][3]=114;
  servo.movimento_topo[2][3]=69;

  servo.movimento_base[2][5]=126;
  servo.movimento_topo[2][5]=65;

  servo.movimento_base[2][7]=136;
  servo.movimento_topo[2][7]=67;

  servo.movimento_base[3][0]=98;
  servo.movimento_topo[3][0]=70;

  servo.movimento_base[3][2]=112;
  servo.movimento_topo[3][2]=61;

  servo.movimento_base[3][4]=127;
  servo.movimento_topo[3][4]=55;

  servo.movimento_base[3][6]=138;
  servo.movimento_topo[3][6]=53;
  
  servo.movimento_base[4][1]=109;
  servo.movimento_topo[4][1]=52;

  servo.movimento_base[4][3]=125;
  servo.movimento_topo[4][3]=45;

  servo.movimento_base[4][5]=137;
  servo.movimento_topo[4][5]=42;

  servo.movimento_base[4][7]=152;
  servo.movimento_topo[4][7]=44;

  servo.movimento_base[5][0]=102;
  servo.movimento_topo[5][0]=48;

  servo.movimento_base[5][2]=118;
  servo.movimento_topo[5][2]=39;

  servo.movimento_base[5][4]=134;
  servo.movimento_topo[5][4]=32;

  servo.movimento_base[5][6]=150;
  servo.movimento_topo[5][6]=31;

  servo.movimento_base[6][1]=111;
  servo.movimento_topo[6][1]=33;

  servo.movimento_base[6][3]=129;
  servo.movimento_topo[6][3]=24;

  servo.movimento_base[6][5]=148;
  servo.movimento_topo[6][5]=20;

  servo.movimento_base[6][7]=164;
  servo.movimento_topo[6][7]=22;

  servo.movimento_base[7][0]=102;
  servo.movimento_topo[7][0]=28;

  servo.movimento_base[7][2]=120;
  servo.movimento_topo[7][2]=20;

  servo.movimento_base[7][4]=140;
  servo.movimento_topo[7][4]=11;

  servo.movimento_base[7][6]=162;
  servo.movimento_topo[7][6]=11;

  // i=1;
  // j=1;
  // printf("\n");
  // while ( i<9 && j<9){
  //   switch (i)
  //   {
  //     case 1:
  //       switch (j)
  //       {
  //       case 2:
  //         servo.movimento_base[i-1][j-1]=78;
  //         servo.movimento_topo[i-1][j-1]=125;
  //         break;
  //       case 4:
  //         servo.movimento_base[i-1][j-1]=101;
  //         servo.movimento_topo[i-1][j-1]=102;
  //         break;
  //       case 6:
  //         servo.movimento_base[i-1][j-1]=114;
  //         servo.movimento_topo[i-1][j-1]=95;
  //         break;
  //       case 8:
  //         servo.movimento_base[i-1][j-1]=122;
  //         servo.movimento_topo[i-1][j-1]=95;
  //         break;
  //       default:
  //         break;
  //       }
  //       break;
  //     case 2:
  //         switch (j)
  //         {
  //           case 1:
  //             servo.movimento_base[i-1][j-1]=85;
  //             servo.movimento_topo[i-1][j-1]=107;
  //             break;
  //           case 3:
  //             servo.movimento_base[i-1][j-1]=105;
  //             servo.movimento_topo[i-1][j-1]=90;
  //             break;
  //           case 5:
  //             servo.movimento_base[i-1][j-1]=116;
  //             servo.movimento_topo[i-1][j-1]=82;
  //             break;
  //           case 7:
  //             servo.movimento_base[i-1][j-1]=126;
  //             servo.movimento_topo[i-1][j-1]=80;
  //             break;
  //           default:
  //             break;
  //         }
  //       break;
  //     case 3:
  //       switch (j)
  //       {
  //         case 2:
  //           servo.movimento_base[i-1][j-1]=98;
  //           servo.movimento_topo[i-1][j-1]=83;
  //           break;
  //         case 4:
  //           servo.movimento_base[i-1][j-1]=112;
  //           servo.movimento_topo[i-1][j-1]=74;
  //           break;
  //         case 6:
  //           servo.movimento_base[i-1][j-1]=126;
  //           servo.movimento_topo[i-1][j-1]=65;
  //           break;
  //         case 8:
  //           servo.movimento_base[i-1][j-1]=136;
  //           servo.movimento_topo[i-1][j-1]=37;
  //           break;
  //         default:
  //         break;
  //       }
  //       break;
  //     case 4:
  //       switch (j)
  //       {
  //         case 1:
  //           servo.movimento_base[i-1][j-1]=97;
  //           servo.movimento_topo[i-1][j-1]=77;
  //           break;
  //         case 3:
  //           servo.movimento_base[i-1][j-1]=110;
  //           servo.movimento_topo[i-1][j-1]=65;
  //           break;
  //         case 5:
  //           servo.movimento_base[i-1][j-1]=124;
  //           servo.movimento_topo[i-1][j-1]=58;
  //           break;
  //         case 7:
  //           servo.movimento_base[i-1][j-1]=140;
  //           servo.movimento_topo[i-1][j-1]=56;
  //           break;
  //         default:
  //         break;
  //       }
  //       break;
  //     case 5:
  //       switch (j)
  //       {
  //         case 2:
  //           servo.movimento_base[i-1][j-1]=102;
  //           servo.movimento_topo[i-1][j-1]=60;
  //           break;
  //         case 4:
  //           servo.movimento_base[i-1][j-1]=122;
  //           servo.movimento_topo[i-1][j-1]=50;
  //           break;
  //         case 6:
  //           servo.movimento_base[i-1][j-1]=137;
  //           servo.movimento_topo[i-1][j-1]=44;
  //           break;
  //         case 8:
  //           servo.movimento_base[i-1][j-1]=152;
  //           servo.movimento_topo[i-1][j-1]=44;
  //           break;
  //         default:
  //           break;
  //       }
  //       break;
  //     case 6:
  //       switch (j)
  //       {
  //         case 1:
  //           servo.movimento_base[i-1][j-1]=99;
  //           servo.movimento_topo[i-1][j-1]=56;
  //           break;
  //         case 3:
  //           servo.movimento_base[i-1][j-1]=117;
  //           servo.movimento_topo[i-1][j-1]=44;
  //           break;
  //         case 5:
  //           servo.movimento_base[i-1][j-1]=134;
  //           servo.movimento_topo[i-1][j-1]=37;
  //           break;
  //         case 7:
  //           servo.movimento_base[i-1][j-1]=149;
  //           servo.movimento_topo[i-1][j-1]=33;
  //           break;
  //         default:
  //           break;
  //       }
  //       break;
  //     case 7:
  //       switch (j)
  //       {
  //         case 2:
  //           servo.movimento_base[i-1][j-1]=112;
  //           servo.movimento_topo[i-1][j-1]=42;
  //           break;
  //         case 4:
  //           servo.movimento_base[i-1][j-1]=126;
  //           servo.movimento_topo[i-1][j-1]=29;
  //           break;
  //         case 6:
  //           servo.movimento_base[i-1][j-1]=144;
  //           servo.movimento_topo[i-1][j-1]=23;
  //           break;
  //         case 8:
  //           servo.movimento_base[i-1][j-1]=164;
  //           servo.movimento_topo[i-1][j-1]=22;
  //           break;
  //         default:
  //         break;
  //       }
  //       break;
  //     case 8:
  //       switch (j)
  //       {
  //         case 1:
  //           servo.movimento_base[i-1][j-1]=100;
  //           servo.movimento_topo[i-1][j-1]=42;
  //           break;
  //         case 3:
  //           servo.movimento_base[i-1][j-1]=115;
  //           servo.movimento_topo[i-1][j-1]=27;
  //           break;
  //         case 5:
  //           servo.movimento_base[i-1][j-1]=135;
  //           servo.movimento_topo[i-1][j-1]=16;
  //           break;
  //         case 7:
  //           servo.movimento_base[i-1][j-1]=160;
  //           servo.movimento_topo[i-1][j-1]=11;
  //           break;
  //         default:
  //         break;
  //       }
  //       break;
  //     default:
  //       break;
  //   }
  //   //printf("%d;%d; [%d ; %d] \n",servo.movimento_base[i-1][j-1],servo.movimento_topo[i-1][j-1], i-1, j-1);
  //   j=j+1;
  //   if(j>8 && i<=7){
  //     i=i+1;
  //     j=1;
  //   }
  //   else if (j>8 && i==8){
  //     j=8;
  //     break;
  //   }
  // }
}

//move o braço para as coordenadas
uint8_t move_servo_AtoB (uint8_t Xinicial, uint8_t Yinicial, uint8_t Xfinal, uint8_t Yfinal){
  //Falta definir pos para colocar peças comidas
  if(Xinicial > 10 && Yinicial > 10){
    move_servo_base(Xinicial);
    move_servo_topo(Yinicial);
  }
  else{
    move_servo_base(servo.movimento_base[Xinicial/*-1*/][Yinicial/*-1*/]);
    move_servo_topo(servo.movimento_topo[Xinicial/*-1*/][Yinicial/*-1*/]);
  }
  // printf("1\n");
  _delay_ms(2000);
  move_servo_ponta(1);
  // printf("2\n");
  _delay_ms(500);
  iman(1);
  // printf("3\n");
  _delay_ms(500);
  move_servo_ponta(0);
  _delay_ms(500);

  if(Xfinal > 10 && Yfinal > 10){
    move_servo_base(Xfinal);
    move_servo_topo(Yfinal);
  }
  else{
    move_servo_base(servo.movimento_base[Xfinal/*-1*/][Yfinal/*-1*/]);
    move_servo_topo(servo.movimento_topo[Xfinal/*-1*/][Yfinal/*-1*/]);
  }

  // printf("4\n");
  _delay_ms(2000);
  move_servo_ponta(1);
  // printf("5\n");
  _delay_ms(500);
  iman(0);
  // printf("6\n");
  _delay_ms(500);
  move_servo_ponta(0);
  _delay_ms(1500);
  // printf("fim");
  // move_servo_base(posicao descanso);
  // move_servo_topo(posicao descanso);
  return 1;
}
