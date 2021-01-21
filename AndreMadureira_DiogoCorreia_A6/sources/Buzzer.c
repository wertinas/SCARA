#include <avr/io.h>
#include <util/delay.h>
#include "Buzzer.h"


void t2_set_prescaler(enum t2_prescaler ps){
  TCCR2B = ps;
}
 
uint16_t t2_get_prescaler_rate(enum t2_prescaler ps){
  
  switch(ps){
    case T2_PRESCALER_1:
      return 1;
      break;
    case T2_PRESCALER_8:
      return 8;
      break;
    case T2_PRESCALER_32:
      return 32;
      break;
    case T2_PRESCALER_64:
      return 64;
      break;
    case T2_PRESCALER_128:
      return 128;
      break;
    case T2_PRESCALER_256:
      return 256;
      break;
    case T2_PRESCALER_1024:
      return 1024;
      break;
  }
  return 0;
}

unsigned long div_round(unsigned long d, unsigned long q)
{
    return (d + (q/2)) / q;
}

void t2_set_ctc_a(uint16_t hz,uint32_t timer_freq){

    OCR2A = div_round(timer_freq, hz*2) - 1 ;
    TCCR2A = (1 << COM2A0) | (1 << WGM21);
}
 
void tone(uint16_t hz){

  if(hz == 0){
    OCR2A = 0;
  } 
  else{
    t2_set_ctc_a(hz,FREQ_TIMER);
  } 
}

void GameOfThrones(void) {
  for(int i=0; i<4; i++) {
    tone(NOTE_G4);
    _delay_ms(500);
    tone(REST);

    tone(NOTE_C4);
    _delay_ms(500);
    tone(REST);

    tone(NOTE_DS4);
    _delay_ms(250);
    tone(REST);

    tone(NOTE_F4);
    _delay_ms(250);
    tone(REST);
  }

  for(int i=0; i<4; i++){
    tone(NOTE_G4);
    _delay_ms(500);
    tone(REST);


    tone(NOTE_C4);
    _delay_ms(500);
    tone(REST);


    tone(NOTE_E4);
    _delay_ms(250);
    tone(REST);


    tone(NOTE_F4);
    _delay_ms(250);
    tone(REST);

  }

    tone(NOTE_G4);
    _delay_ms(1500);
    tone(REST);

    tone(NOTE_C4);
    _delay_ms(1500);
    tone(REST);

  //
    tone(NOTE_E4);
    _delay_ms(250);
    tone(REST);

    tone(NOTE_F4);
    _delay_ms(250);
    tone(REST);
  //
    tone(NOTE_G4);
    tone(REST);
    _delay_ms(1000);
    tone(REST);
    
    tone(NOTE_C4);
    _delay_ms(1000);
    tone(REST);

    tone(NOTE_DS4);
    _delay_ms(250);
    tone(REST);

    tone(NOTE_F4);
    _delay_ms(250);
    tone(REST);
    
  for(int i=0; i<3; i++) {
    
    tone(NOTE_D4);
    _delay_ms(500);
    tone(REST);
    
    tone(NOTE_G3);
    _delay_ms(500);
    tone(REST);

    tone(NOTE_AS3);
    _delay_ms(250);
    tone(REST);

    tone(NOTE_C4);
    _delay_ms(250);
    tone(REST);    
    
  }
  


      tone(NOTE_D4);
      _delay_ms(1500);
      tone(REST);
      
      tone(NOTE_F4);
      _delay_ms(1500);
      tone(REST);

      tone(NOTE_AS3);
      _delay_ms(1000);
      tone(REST);

      tone(NOTE_DS4);
      _delay_ms(250);
      tone(REST);

      tone(NOTE_D4);
      _delay_ms(250);
      tone(REST);

      tone(NOTE_F4);
      _delay_ms(1000);
      tone(REST);

      tone(NOTE_AS3);
      _delay_ms(1000);
      tone(REST);

      tone(NOTE_DS4);
      _delay_ms(250);
      tone(REST);

      tone(NOTE_D4);
      _delay_ms(250);
      tone(REST);

      tone(NOTE_C4);
      _delay_ms(500);

  for(int i=0; i<3; i++) {
      tone(NOTE_GS3);
      _delay_ms(250);

      tone(NOTE_AS3);
      _delay_ms(250);

      tone(NOTE_C4);
      _delay_ms(500);
      tone(REST);

      tone(NOTE_F3);

      _delay_ms(500);
      tone(REST);
  }

    tone(NOTE_G4);
    _delay_ms(1000);
    tone(REST);

    tone(NOTE_C4);
    _delay_ms(1000);
    tone(REST);

    tone(NOTE_DS4);;
    _delay_ms(250);
    tone(REST);;

    tone(NOTE_F4);
    _delay_ms(250);
    tone(REST);

    tone(NOTE_G4);
    _delay_ms(1000);
    tone(REST);

    tone(NOTE_C4);
    _delay_ms(1000);
    tone(REST);

    tone(NOTE_DS4);;
    _delay_ms(250);
    tone(REST);;

    tone(NOTE_F4);
    _delay_ms(250);
    tone(REST);

    tone(NOTE_D4);
    _delay_ms(500);
    tone(REST);

  for(int i=0; i<4; i++) {
    tone(NOTE_G3);
    _delay_ms(500);
    tone(REST);

    tone(NOTE_AS3);
    _delay_ms(250);
    tone(REST);;

    tone(NOTE_C4);
    _delay_ms(250);
    tone(REST);

    tone(NOTE_D4);
    _delay_ms(500);
    tone(REST);
  }
}

void EyeOfTheTiger(void){
  int i = 0;
  while(i <3){

  tone(988);
  _delay_ms(150);
  tone(REST);
  _delay_ms(1000);
  tone(988);
    _delay_ms(150);
  tone(REST);
  _delay_ms(150);
  tone(880);
    _delay_ms(250);
  tone(REST);
  _delay_ms(150);
  tone(988);
    _delay_ms(250);
  tone(REST);
  _delay_ms(1000);
 
  tone(988);
    _delay_ms(250);
  tone(REST);
  _delay_ms(150);
  tone(880);
  _delay_ms(250);
  tone(REST);
  _delay_ms(150);
  tone(988);
    _delay_ms(250);
  tone(REST);
  _delay_ms(1000);
 
  tone(988);
    _delay_ms(250);
  tone(REST);
  _delay_ms(150);
  tone(880);
    _delay_ms(250);
  tone(REST);
  _delay_ms(150);
  tone(784);
    _delay_ms(250);
  tone(REST);
  _delay_ms(1000);
  tone(880);
    _delay_ms(250);
  tone(REST);
  _delay_ms(1000);
  //replays loop
  i++;
  }
}

void buzzer_init(void){
        DDRB |= (1 << PB3);
}

// int main(void)
// {
//     unsigned long timer_freq;
//     enum t2_prescaler ps = T2_PRESCALER_256;
 
//     // DDRD |= _BV(DDD6);
//     DDRB |= (1 << PB3);
//     t2_set_prescaler(ps);
//     timer_freq = div_round(F_CPU, t2_get_prescaler_rate(ps));
 
//     while(1)
//     {
//         t2_set_ctc_a(440, timer_freq);
//         _delay_ms(500);
//         t2_set_ctc_a(880, timer_freq);
//         _delay_ms(500);

//         // for (int i = 0; i < 20000; i+=5)
//         // {
//         //   t2_set_ctc_a(i,timer_freq);
//         //   _delay_ms(250);
//         // }
        
//     }
//     return 0;
// }
