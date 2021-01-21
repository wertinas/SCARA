#include <avr\io.h>
#include <avr\interrupt.h>

typedef struct {
    int movimento_base[8][8];
    int movimento_topo[8][8];
} movimentos;

typedef struct {
	int X;
	int Y;
    int player;
}Coordenadas;

void io_init(void);

void servo_tc0_PWM_init(void);

void servo_tc1_PWM_init(void);

void move_servo_base(uint8_t angle);

void move_servo_topo(uint8_t angle);

void move_servo_ponta(uint8_t sobeDesce);

void iman(uint8_t state);

void calculo_pos(void);

uint8_t move_servo_AtoB(uint8_t Xincial, uint8_t Yinicial, uint8_t Xfinal, uint8_t Yfinal);
