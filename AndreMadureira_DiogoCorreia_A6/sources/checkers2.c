/************************************************************************************
 * Projeto Damas com SCARA                                                         	*
 *                                                           					   	*
 * Membros ==> | Diogo Correia | André Madureira |									*
 * 			   |   up201705099 |   up201706076   |									*
 * Turma   ==> A6                                                               	*
 ************************************************************************************/

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "serial_printf.h"
#include "Inputs.h"
#include "Buzzer.h"
#include "Move_Servo.h"
#include <stdlib.h>
#include "timer_tools.h"

#define MAXPECAS 8
#define LIXOP1X 160
#define LIXOP1Y 60
#define LIXOP2X 80
#define LIXOP2Y	80
#define MAXPLAYS 100

typedef struct {
	uint8_t Xinit;
	char Yinit;
	uint8_t Xfinal;
	char Yfinal;
} Hist;

Hist RankJogadas[MAXPLAYS];

uint8_t EEMEM linha_inicio[MAXPLAYS];
uint8_t EEMEM coluna_inicio[MAXPLAYS];
uint8_t EEMEM linha_fim[MAXPLAYS];
uint8_t EEMEM coluna_fim[MAXPLAYS];



uint8_t posPecaRetirar = 0, num_play = 0, minJogadas = 100, jogar = 1;
Coordenadas pecaRetirar[MAXPECAS];

/* initializing the board array with pieces as a global array
  -1 for boxes which can not be used
   1 for pieces of player1
   2 for pieces of player2
   0 for unsed usable boxes */
signed char boxes[8][8] = { {-1, 1,-1, 1,-1, 1,-1, 1 }, 
					{ 1,-1, 1,-1, 1,-1, 1,-1 }, 
					{-1, 1,-1, 1,-1, 1,-1, 1 }, 
					{ 0,-1, 0,-1, 0,-1, 0,-1 }, 
					{-1, 0,-1, 0,-1, 0,-1, 0 }, 
					{ 2,-1, 2,-1, 2,-1, 2,-1 }, 
					{-1, 2,-1, 2,-1, 2,-1, 2 }, 
					{ 2,-1, 2,-1, 2,-1, 2,-1 } };

signed char new_tab[8][8]= { {-1, 1,-1, 1,-1, 1,-1, 1 }, 
					{ 1,-1, 1,-1, 1,-1, 1,-1 }, 
					{-1, 1,-1, 1,-1, 1,-1, 1 }, 
					{ 0,-1, 0,-1, 0,-1, 0,-1 }, 
					{-1, 0,-1, 0,-1, 0,-1, 0 }, 
					{ 2,-1, 2,-1, 2,-1, 2,-1 }, 
					{-1, 2,-1, 2,-1, 2,-1, 2 }, 
					{ 2,-1, 2,-1, 2,-1, 2,-1 } };

// printing board
void print_board(signed char boxes[8][8]){
	printf("\nTabuleiro:\n");
	uint8_t i,j;
	char str[9] = {'A','B','C','D','E','F','G','H'};
	printf("\n |1|2|3|4|5|6|7|8|\n");
	for ( j = 0; j < 8; j++){
		for(i = 0; i < 17; i++){
			printf("%c",'-');
		}
		printf("\n");
		printf("%c",str[j]);
		for(i = 0; i < 8; i++){
			if(boxes[j][i] == -1) printf("%c%c",'|','#');
			else if(boxes[j][i] == 0) printf("%c%c",'|',' ');
			else printf("%c%d",'|',boxes[j][i]);
		}
		printf("%c\n",'|');
	}
	for(i = 0; i < 17; i++){
		printf("%c",'-');
	}							
}




/* * * * * * * * * * * * * * * * * * Funcoes : Movimentos e logica de jogo * * * * * * * * * * * * * * * * * */
void playerTurn(uint8_t player);
uint8_t comeCome(Coordenadas Bolo[], uint8_t *fatias){
	uint8_t i;

	if(*fatias == 0) return 1;
	// printf("Fatias: %d",*fatias);
	for(i = 0; i < *fatias; i++){
		// printf("Comidelas yo %d\n",i);

		// printf("bolo[%d].x = %d bolo[%d].y = %d\n",i,Bolo[i].X,i,Bolo[i].Y);
		(Bolo[i].player == 1) ? move_servo_AtoB(Bolo[i].X,Bolo[i].Y,LIXOP1X,LIXOP1Y) : move_servo_AtoB(Bolo[i].X,Bolo[i].Y,LIXOP2X,LIXOP2Y);
		//tempo de a peca ja ter sido deslocada
	}
	*fatias = 0;
	return 1;

}

//function for checking new kings
void checkKing(){
	for(uint8_t i=0;i<8;i++){
		if(boxes[7][i]==1){				//red piece becomes king if it reaches to last row
			// move_servo_AtoB(7,i,LIXOP1X,LIXOP1Y);
			// _delay_ms(500);
			move_servo_AtoB(LIXOP1X,LIXOP1Y,7,i);
			boxes[7][i]=3;
		}           

		if(boxes[0][i]==2){	           //white piece becomes king if it reaches to first row
		
			// move_servo_AtoB(0,i,LIXOP2X,LIXOP2Y);
			// _delay_ms(500);
			move_servo_AtoB(LIXOP2X,LIXOP2Y,0,i);
			boxes[0][i]=4;
		}
	}
}


uint8_t countPieces(uint8_t player){
	uint8_t count=0;
	/* start counting pieces of player */
	for(uint8_t i=0;i<8;i++){
		for(uint8_t j=0;j<8;j++){
			if(boxes[i][j]==player || boxes[i][j]==(player+2))        
				count++;
		}	
	} /* End counting pieces of player  */
	printf("\n* * * * * * * * * * * * * * *\n");
	printf(	 "|  Player %d  Pieces =  %02d   |\n",player,count);
	printf(	 "* * * * * * * * * * * * * * *\n");
	return count;
}


//defining the move down function
uint8_t moveDown(uint8_t type,uint8_t row,uint8_t col,uint8_t newRow,uint8_t newCol){
	/*checking for a valid move*/
		//primeiras duas condicoes ja verificadas, considerar remove las
	//false move if new place is not empty	
	if(boxes[newRow][newCol]!=0){
		printf("\n[moveDown] Espaco ja ocupado\n");
		return 0;
	}

	//false move if player piece is not placed at input position 
	else if(boxes[row][col]==-1 || boxes[row][col]==0 ){	
		printf("\n[moveDown] A sua peca nao esta nessa posicao\n");
				
		return 0;
	}

	//valid move if new place is in diagnol 
	else if ((newRow == row + 1) && (newCol == col - 1 || newCol == col + 1)){	
		boxes[newRow][newCol] = boxes[row][col];
		boxes[row][col] = 0;
		move_servo_AtoB(row,col,newRow,newCol);
		return 1;
	}

	//checker by player if checks opponent piece in diagnol
	else if ((newRow == row + 2) && (newCol == col - 2 || newCol == col + 2)){
			if ((boxes[newRow - 1][newCol - 1] == type-1 || boxes[newRow - 1][newCol - 1] == type-3 || boxes[newRow - 1][newCol - 1] == type+1 || boxes[newRow - 1][newCol - 1] == type+3) && (boxes[newRow - 1][newCol - 1] !=0) && newCol>col){
				/*******************************/
				//comidelas em ação
				pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 1:2);

				pecaRetirar[posPecaRetirar].X = newRow - 1;
				pecaRetirar[posPecaRetirar++].Y = newCol-1;

				boxes[newRow - 1][newCol - 1] = 0;
				/*******************************/

				boxes[newRow][newCol] = boxes[row][col];
				boxes[row][col] = 0;
				move_servo_AtoB(row,col,newRow,newCol);
				return 1;
			}
			else if ((boxes[newRow - 1][newCol + 1] == type-1 || boxes[newRow - 1][newCol + 1] == type-3 || boxes[newRow - 1][newCol + 1] == type+1 || boxes[newRow - 1][newCol + 1] == type+3) && (boxes[newRow - 1][newCol + 1] !=0) && newCol<col){
				/*******************************/
				//comidelas em ação
				pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 1:2);

				pecaRetirar[posPecaRetirar].X = newRow - 1;
				pecaRetirar[posPecaRetirar++].Y = newCol + 1;

				boxes[newRow - 1][newCol + 1] = 0;
				/*********************************/

				boxes[newRow][newCol] = boxes[row][col];
				boxes[row][col] = 0;
				move_servo_AtoB(row,col,newRow,newCol);
				return 1;
			}
	}

	//double checker by player
	else if((newRow == row + 4) && (newCol == col - 4 || newCol == col + 4 || newCol==col)){	
			if(newCol == col + 4){
					if(((boxes[row+1][col+1]==type-1 || boxes[row+1][col+1]==type+1 ||  boxes[row+1][col+1]==type+3 || boxes[row+1][col+1]==type-3) && boxes[row+1][col+1]!=0) && boxes[row+2][col+2]==0 && ((boxes[row+3][col+3]==type-1 || boxes[row+3][col+3]==type+1 || boxes[row+3][col+3]==type+3 ||  boxes[row+3][col+3]==type-3) && boxes[row+3][col+3]!=0)){
						/*******************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row + 1;
						pecaRetirar[posPecaRetirar++].Y = col + 1;

						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row + 3;
						pecaRetirar[posPecaRetirar++].Y = col + 3;
						
						
						boxes[row+1][col+1]   = 0;
						boxes[row+3][col+3]   = 0;
						/***********************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
					else{
						printf("[moveDown] doublechecker1\n");
						return 0;
					}
			}
			else if(newCol == col - 4){
					if(((boxes[row+1][col-1]==type-1 || boxes[row+1][col-1]==type+1 || boxes[row+1][col-1]==type-3 || boxes[row+1][col-1]==type+3)&&boxes[row+1][col-1]!=0) && boxes[row+2][col-2]==0 && ((boxes[row+3][col-3]==type-1 || boxes[row+3][col-3]==type+1 || boxes[row+3][col-3]==type-3 || boxes[row+3][col-3]==type+3)&&boxes[row+3][col-3]!=0)){
						
						/*******************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row + 1;
						pecaRetirar[posPecaRetirar++].Y = col-1;

						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = newRow - 1;
						pecaRetirar[posPecaRetirar++].Y = newCol-1;
						
						boxes[row+1][col-1]   = 0;
						boxes[row+3][col-3]   = 0;
						/************************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
					else{
						printf("[moveDown] doublechecker2\n");
						return 0;
					}
			}
			else if(newCol == col){
					if(boxes[row+1][col+1]!=0 && (boxes[row+1][col+1]==type-1 || boxes[row+1][col+1]==type-3 || boxes[row+1][col+1]==type+3 || boxes[row+1][col+1]==type+1) && ((boxes[row+3][col+1]==type-1 || boxes[row+3][col+1]==type-3 ||  boxes[row+3][col+1]==type+3 || boxes[row+3][col+1]==type+1)&&boxes[row+3][col+1]!=0) && boxes[row+2][col+2]==0){
						
						/*******************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row + 1;
						pecaRetirar[posPecaRetirar++].Y = col + 1;
						
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row + 3;
						pecaRetirar[posPecaRetirar++].Y = col + 1;
						
						boxes[row+1][col+1]   = 0;
						boxes[row+3][col+1]   = 0;
						/***************************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
					else if(boxes[row+1][col-1]!=0 && (boxes[row+1][col-1]==type-1 || boxes[row+1][col-1]==type+3 || boxes[row+1][col-1]==type-3 || boxes[row+1][col-1]==type+1)  && (boxes[row+3][col-1]!=0 && (boxes[row+3][col-1]==type-1 || boxes[row+3][col-1]==type+3 || boxes[row+3][col-1]==type-3 || boxes[row+3][col-1]==type+1) && boxes[row+2][col-2]==0)){
						
						/*******************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row + 1;
						pecaRetirar[posPecaRetirar++].Y = col - 1;

						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row + 3;
						pecaRetirar[posPecaRetirar++].Y = col - 1;
						
						boxes[row+1][col-1]   = 0;
						boxes[row+3][col-1]   = 0;
						/*********************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
					else{
						printf("[moveDown] doublechecker3\n");
						return 0;
					}
			}
			else{
				printf("[moveDown] doublechecker4\n");

				return 0;
			}
	}
	printf("[moveDown] fim \n");

	return 0;
}

//defining the move up function
uint8_t moveUp(uint8_t type, uint8_t row,uint8_t col,uint8_t newRow,uint8_t newCol){
	/*checking for a valid move*/
		
	//false move if new place is not empty
	if(boxes[newRow][newCol]!=0){
		printf("\n[moveUp] Espaco ja ocupado\n");
		return 0;
	}

	//false move if player piece is not placed at input position
	else if(boxes[row][col]==-1 || boxes[row][col]==0 ){
		printf("\n[moveUp] A sua peca nao se encontra nessa posicao\n");
					
		return 0;
	}

	//valid move if new place is in diagnol 
	else if ((newRow == row - 1) && (newCol == col - 1 || newCol == col + 1)){
		boxes[newRow][newCol] = boxes[row][col];
		boxes[row][col] = 0;
		move_servo_AtoB(row,col,newRow,newCol);
		return 1;
	}

	//checker by player if checks opponent piece in diagnol
	else if ((newRow == row - 2) && (newCol == col - 2 || newCol == col + 2)){
			if (boxes[newRow + 1][newCol - 1] !=0 && (boxes[newRow + 1][newCol - 1] == type-1 || boxes[newRow + 1][newCol - 1] == type-3 || boxes[newRow + 1][newCol - 1] == type+1) && newCol>col){
				
				/*******************************/
				//comidelas em ação
				pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

				pecaRetirar[posPecaRetirar].X = newRow + 1;
				pecaRetirar[posPecaRetirar++].Y = newCol - 1;
				
				boxes[newRow + 1][newCol - 1] = 0;
				/*********************************/

				boxes[newRow][newCol] = boxes[row][col];
				boxes[row][col] = 0;
				move_servo_AtoB(row,col,newRow,newCol);
				return 1;
			}
			else if (boxes[newRow + 1][newCol + 1] !=0 &&(boxes[newRow + 1][newCol + 1] == type-1 || boxes[newRow + 1][newCol + 1] == type-3 || boxes[newRow + 1][newCol + 1] == type+1) && newCol<col){
				
				/**********************************/
				//comidelas em ação
				pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

				pecaRetirar[posPecaRetirar].X = newRow + 1;
				pecaRetirar[posPecaRetirar++].Y = newCol + 1;

				boxes[newRow + 1][newCol + 1] = 0;
				/*************************************/
				boxes[newRow][newCol] = boxes[row][col];
				boxes[row][col] = 0;
				move_servo_AtoB(row,col,newRow,newCol);
				return 1;
			}
	}

	//double checker by player
	else if((newRow == row - 4) && (newCol == col - 4 || newCol == col + 4 || newCol==col)){
			if(newCol == col + 4){
					if(boxes[row-1][col+1]!=0 && (boxes[row-1][col+1]==type-1 || boxes[row-1][col+1]==type+1 || boxes[row-1][col+1]==type+3 || boxes[row-1][col+1]==type-3) && boxes[row-2][col+2]==0 && (boxes[row-3][col+3]==type-1 || boxes[row-3][col+3]==type-3 || boxes[row-3][col+3]==type+3 || boxes[row-3][col+3]==type+1)){
						
						/**********************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 1;
						pecaRetirar[posPecaRetirar++].Y = col + 1;

						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 3;
						pecaRetirar[posPecaRetirar++].Y = col + 3;
						
						boxes[row-1][col+1]   = 0;
						boxes[row-3][col+3]   = 0;
						/**************************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
					else{
						printf("[moveUp] doublechecker1\n");
						return 0;
					}
			}
			else if(newCol == col - 4){
					if(boxes[row-1][col-1]!=0 && (boxes[row-1][col-1]==type-1 || boxes[row-1][col-1]==type+1 || boxes[row-1][col-1]==type+3 || boxes[row-1][col-1]==type-3) && boxes[row-2][col-2]==0 && (boxes[row-3][col-3]==type-1 || boxes[row-3][col-3]==type+3 || boxes[row-3][col-3]==type-3 || boxes[row-3][col-3]==type+1)){
						
						/**********************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 1;
						pecaRetirar[posPecaRetirar++].Y = col - 1;
						
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 3;
						pecaRetirar[posPecaRetirar++].Y = col - 3;
						
						boxes[row-1][col-1]   = 0;
						boxes[row-3][col-3]   = 0;
						/********************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
					else{
						printf("[moveUp] doublechecker2\n");
						return 0;
					}
			}
			else if(newCol == col){
				if((boxes[row-1][col-1]!=0 && (boxes[row-1][col-1]==type-1 || boxes[row-1][col-1]==type-3 ||boxes[row-1][col-1]==type+3 || boxes[row-1][col-1]==type+1)) && boxes[row-2][col-2]==0 && (boxes[row-3][col-1]==type-1 || boxes[row-3][col-1]==type-3 || boxes[row-3][col-1]==type+3 || boxes[row-3][col-1]==type+1)){
						
						/**********************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 1;
						pecaRetirar[posPecaRetirar++].Y = col - 1;

						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 3;
						pecaRetirar[posPecaRetirar++].Y = col - 1;
						
						boxes[row-1][col-1]   = 0;
						boxes[row-3][col-1]   = 0;
						/***************************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
				else if((boxes[row-1][col+1]!=0 && (boxes[row-1][col+1]==type-1 || boxes[row-1][col+1]==type-3 || boxes[row-1][col+1]==type+3 || boxes[row-1][col+1]==type+1)) && (boxes[row-3][col+1]!=0 && (boxes[row-3][col+1]==type-1 || boxes[row-3][col+1]==type+1 || boxes[row-3][col+1]==type-3 ||boxes[row-3][col+1]==type+3) && boxes[row-2][col+2]==0)){
						
						/**********************************/
						//comidelas em ação
						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 1;
						pecaRetirar[posPecaRetirar++].Y = col + 1;

						pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

						pecaRetirar[posPecaRetirar].X = row - 3;
						pecaRetirar[posPecaRetirar++].Y = col + 1;

						/***********************************/

						boxes[row-1][col+1]   = 0;
						boxes[row-3][col+1]   = 0;
						/***********************************/
						boxes[newRow][newCol] = boxes[row][col];
						boxes[row][col]		  = 0;
						move_servo_AtoB(row,col,newRow,newCol);
						return 1;
					}
					else{
						printf("[moveUp] doublechecker3\n");
						return 0;
					}
			}
			else{
				printf("[moveUp] doublechecker4\n");
				return 0;
			}
	}
	printf("[moveUp] fim\n");
	return 0;
}

uint8_t backFor(uint8_t type, uint8_t row,uint8_t col,uint8_t newRow,uint8_t newCol){
	//forward backword double check by king
	
	if(newCol == col - 4 || newCol == col + 4){
		if(newCol == col + 4){
				//right up
				if((row!= 0 && row!=1) && boxes[row-2][col+2]==0 && boxes[row-1][col+1]!=0 && (boxes[row-1][col+1]==type-1 || boxes[row-1][col+1]==type+1 || boxes[row-1][col+1]==type-3 || boxes[row-1][col+1]==type+3)  && (boxes[row-1][col+3]==type-1 || boxes[row-1][col+3]==type+1 || boxes[row-1][col+3]==type-3) && ( boxes[row-1][col+3]!= 0 )){
					
					/***********************************/
					//comidelas em acao
					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row - 1;
					pecaRetirar[posPecaRetirar++].Y = col + 1;

					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row - 1;
					pecaRetirar[posPecaRetirar++].Y = col + 3;
					
					boxes[row-1][col+1]=0;
					boxes[row-1][col+3]=0;
					/*********************************/
					
					boxes[newRow][newCol]=boxes[row][col];
					boxes[row][col]=0;
					move_servo_AtoB(row,col,newRow,newCol);
					return 1;
				}
				//right down
				else if((row!= 6 && row!=7) && boxes[row+2][col+2]==0 && boxes[row+1][col+1]!=0 && (boxes[row+1][col+1]==type-1 || boxes[row+1][col+1]==type+1 || boxes[row+1][col+1]==type-3 || boxes[row+1][col+1]==type+3)  && (boxes[row+1][col+3]==type-1 || boxes[row+1][col+3]==type+1 || boxes[row+1][col+3]==type-3) && (boxes[row+1][col+3]!=0)){
					
					/***********************************/
					//comidelas em acao
					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row + 1;
					pecaRetirar[posPecaRetirar++].Y = col + 1;

					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row + 1;
					pecaRetirar[posPecaRetirar++].Y = col + 3;
					
					boxes[row+1][col+1]=0;
					boxes[row+1][col+3]=0;
					/*************************************/
					
					boxes[newRow][newCol]=boxes[row][col];
					boxes[row][col]=0;
					move_servo_AtoB(row,col,newRow,newCol);
					return 1;
				}
				else{
					printf("\n[backFor] right\n");
					return 0;
				}
			}
		else if(newCol == col - 4){
				//left up
				if((row!= 0 && row!=1) && boxes[row-2][col-2]==0 && (boxes[row-1][col-1]!=0) && (boxes[row-1][col-1]==type-1 || boxes[row-1][col-1]==type+1 || boxes[row-1][col-1]==type-3 || boxes[row-1][col-1]==type+3)  && (boxes[row-1][col-3]==type-1 || boxes[row-1][col-3]==type+1 || boxes[row-1][col-3]==type-3) && (boxes[row-1][col-3]!=0)){
					
					/***********************************/
					//comidelas em acao
					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row - 1;
					pecaRetirar[posPecaRetirar++].Y = col - 1;

					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row - 1;
					pecaRetirar[posPecaRetirar++].Y = col - 3;
					
					boxes[row-1][col-1]=0;
					boxes[row-1][col-3]=0;
					/*************************************/
					boxes[newRow][newCol]=boxes[row][col];
					boxes[row][col]=0;
					move_servo_AtoB(row,col,newRow,newCol);
					return 1;
				}
				//left down
				else if((row!= 6 && row!=7) && boxes[row+2][col-2]==0 && (boxes[row+1][col-1]!=0) && (boxes[row+1][col-1]==type-1 || boxes[row+1][col-1]==type+1 || boxes[row+1][col-1]==type-3 || boxes[row+1][col-1]==type+3)  && (boxes[row+1][col-3]==type-1 || boxes[row+1][col-3]==type+1 || boxes[row+1][col-3]==type-3) && (boxes[row+1][col-3]!=0)){
					
					/***********************************/
					//comidelas em acao
					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row + 1;
					pecaRetirar[posPecaRetirar++].Y = col - 1;

					pecaRetirar[posPecaRetirar].player = ((type == 1 || type == 3) ? 2:1);

					pecaRetirar[posPecaRetirar].X = row + 1;
					pecaRetirar[posPecaRetirar++].Y = col - 3;

					boxes[row+1][col-1]=0;
					boxes[row+1][col-3]=0;
					/***********************************/
					
					boxes[newRow][newCol]=boxes[row][col];
					boxes[row][col]=0;
					move_servo_AtoB(row,col,newRow,newCol);
					return 1;
				}
				else{
					printf("\n[backFor] left\n");
					return 0;
				}
			
			}
	}
	printf("\n[backFor] fim\n");
	return 0;
}

//function to print result
void win(uint8_t result){

	uint8_t i,j;
	if (result == 1){
		printf("Winner: Player 1\n");
		PORTB |= (1 << PB0);
		PORTD &= ~(1 << PD7);
		GameOfThrones();
	}
	else if (result == 2){
		printf("Winner: Player 2\n");	
		PORTD |= (1 << PD7);
		PORTB &= ~(1 << PB0);
		EyeOfTheTiger();
	}

	printf("Jogadas Realizadas:\n");
	for (i = 0; i < num_play ; i++)
		printf("%c%d -> %c%d\n",RankJogadas[i].Xinit,RankJogadas[i].Yinit,RankJogadas[i].Xfinal,RankJogadas[i].Yfinal);


	if(num_play < minJogadas){
		for (i = 0; i < num_play; i++){
			eeprom_update_byte(&(linha_inicio[i]),convChar2Num(RankJogadas[i].Xinit));
			eeprom_update_byte(&(coluna_inicio[i]),RankJogadas[i].Yinit);
			eeprom_update_byte(&(linha_fim[i]),convChar2Num(RankJogadas[i].Xfinal));
			eeprom_update_byte(&(coluna_fim[i]),RankJogadas[i].Yfinal);
			RankJogadas[i].Xinit = 0;
			RankJogadas[i].Yinit = 0;
			RankJogadas[i].Xfinal = 0;
			RankJogadas[i].Yfinal = 0;
		}
		for (i = num_play; i < minJogadas; i++){
			eeprom_write_byte(&(linha_inicio[i]),0);
			eeprom_write_byte(&(coluna_inicio[i]),0);
			eeprom_write_byte(&(linha_fim[i]),0);
			eeprom_write_byte(&(coluna_fim[i]),0);
		}
		printf(" NOVO RECORDE JOGADAS = %d \n",num_play);
		minJogadas = num_play;
	}
	else{
		printf("Historico das jogadas do jogo mais rapido:\n");
		for ( i = 0; i < minJogadas; i++)
		{
			printf("%c%d -> %c%d\n",convNum2Char(eeprom_read_byte(&linha_inicio[i])),
									eeprom_read_byte(&coluna_inicio[i]),
									convNum2Char(eeprom_read_byte(&linha_fim[i])),
									eeprom_read_byte(&coluna_fim[i]));
		}
	}
	printf("----------------------\n");
	printf("NOVO JOGO...\n");

	for ( i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			boxes[i][j]=new_tab[i][j];
		}
	}
	print_board(boxes);
	playerTurn(1);
	
}
//function to convert the row names into integers(row numbers) entered by user
uint8_t convert(char val){
	
	switch(val){
	case 'A':case 'a':
			return 0;
			break;
	case 'B':case 'b':
			return 1;
			break;
	case 'C':case 'c':
			return 2;
			break;
	case 'D':case 'd':
			return 3;
			break;
	case 'E':case 'e':
			return 4;
			break;
	case 'F':case 'f':
			return 5;
			break;
	case 'G':case 'g':
			return 6;
			break;
	case 'H':case 'h':
			return 7;
			break;
	default:
			return -1;
			break;
	}
}

uint8_t noMoves(uint8_t player){
	uint8_t rowLoc=0;
	
	//storing piece locations
	uint8_t pieces[12][3]={0};	//dos 3, os primeiros 2 indicam a posicao e o ultimo se e rei ou peao
	for(uint8_t i=0;i<8;i++){
		for(uint8_t j=0;j<8;j++){
			if(boxes[i][j]==player || boxes[i][j]==player+2){
				pieces[rowLoc][0]=i;
				pieces[rowLoc][1]=j;
				if(boxes[i][j]==player+2){
					pieces[rowLoc][2]=1;
				}
			rowLoc++;
			}
		}
	}

	if(player==1){
		//double check availablity check
		rowLoc=0;
		while(!(pieces[rowLoc][0]==0 && pieces[rowLoc][1]==0) && rowLoc<12){		//searching the array elements
		/*KING moves*/
		if(pieces[rowLoc][2]==1){
			//double check up left
			if((pieces[rowLoc][0]-4>=0 && pieces[rowLoc][1]-4>=0) && boxes[pieces[rowLoc][0]-4][pieces[rowLoc][1]-4]==0 && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==4) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-3]==2 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-3]==4)){
				return 0;
			}
			//double check up right
			else if((pieces[rowLoc][0]-4>=0 && pieces[rowLoc][1]+4<8) && boxes[pieces[rowLoc][0]-4][pieces[rowLoc][1]+4]==0 && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==4) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+3]==2 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+3]==4)){			
				return 0;
			}
		}
		//double check same column up
		if(pieces[rowLoc][0]-4>=0 && boxes[pieces[rowLoc][0]-4][pieces[rowLoc][1]]==0){
			//same column from right
			if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==4) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+1]==4)){
				return 0;
			}
			//same column from left
			if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==4) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-1]==4)){
				return 0;					
			}
		}
		//double check same row
			//to left
			if(boxes[pieces[rowLoc][0]][pieces[rowLoc][1]-4]==0){
				//double check left from up
				if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==4) && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-3]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-3]==4)){
					return 0;
				}
				//double check left from down
				if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==4) && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-3]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-3]==4)){
						return 0;
				}
			}
			//to right
			if(boxes[pieces[rowLoc][0]][pieces[rowLoc][1]+4]==0){
				//double check right from up
				if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==4) && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+3]==2 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+3]==4)){
					return 0;
				}
				//double check right from down
				if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==4) && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+3]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+3]==4)){
						return 0;
				}
			}
			rowLoc++;
		}
		/*SIMPLE piece moves*/
		while(!(pieces[rowLoc][0]==0 && pieces[rowLoc][1]==0) && rowLoc<12){		//searching the array elements
			if(pieces[rowLoc][2]==1 || pieces[rowLoc][2]==0){	
						//double check up left
						if((pieces[rowLoc][0]+4>=0 && pieces[rowLoc][1]-4>=0) && boxes[pieces[rowLoc][0]+4][pieces[rowLoc][1]-4]==0 && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==4) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-3]==2 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-3]==4)){
							return 0;
						}
						//double check up right
						else if((pieces[rowLoc][0]+4>=0 && pieces[rowLoc][1]+4<8) && boxes[pieces[rowLoc][0]+4][pieces[rowLoc][1]+4]==0 && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==4) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+3]==2 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+3]==4)){
							return 0;
						}
						//double check same column up
						else if(pieces[rowLoc][0]+4>=0 && boxes[pieces[rowLoc][0]+4][pieces[rowLoc][1]]==0){
							//same column from right
							if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==4) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+1]==2 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+1]==4)){
								return 0;
							}
							//same column from right
							if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==4) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-1]==2 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-1]==4)){
								return 0;
							}
					}	
				}
				rowLoc++;
		}
		//computer looking for a single check
		rowLoc=0;
			while(!(pieces[rowLoc][0]==0 && pieces[rowLoc][1]==0) && rowLoc<12){
					/*KING MOVES*/
					if(pieces[rowLoc][2]==1){
						//single check up left
						if((pieces[rowLoc][0]-2>=0 && pieces[rowLoc][1]-2>=0) && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==3)){
							return 0;
						}
						//single check up right
						else if((pieces[rowLoc][0]-2>=0 && pieces[rowLoc][1]+2<8) && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==3)){
							return 0;
						}
						/*Simple moves*/
					if(pieces[rowLoc][2]==1 || pieces[rowLoc][2]==0){
							//single check down left
							if((pieces[rowLoc][0]+2<8 && pieces[rowLoc][1]-2>=0) && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==3)){
								return 0;
							}
							//single check down right
							else if((pieces[rowLoc][0]+2<8 && pieces[rowLoc][1]+2<8) && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==3)){
								return 0;
							}	
						}
					}		
			rowLoc++;
			}
			rowLoc=0;
			while(!(pieces[rowLoc][0]==0 && pieces[rowLoc][1]==0) && rowLoc<12){
				/*King Moves*/
				if(pieces[rowLoc][2]==1){
					//moving up
					if(boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==0 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==0){
						return 0;
					}
				}
				/*simple piece moves*/
				if(pieces[rowLoc][2]==1 || pieces[rowLoc][2]==0){
					//moving down
					if(boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==0 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==0){
						return 0;
					}
				}
			rowLoc++;
			}
			return 1;
	}

	if(player==2){
		//double check availablity check
		rowLoc=0;
		while(!(pieces[rowLoc][0]==0 && pieces[rowLoc][1]==0) && rowLoc<12){		//searching the array elements
		
		/*King moves*/
			if(pieces[rowLoc][2]==1 || pieces[rowLoc][2]==0){	
						//double check up left
						if((pieces[rowLoc][0]+4>=0 && pieces[rowLoc][1]-4>=0) && boxes[pieces[rowLoc][0]+4][pieces[rowLoc][1]-4]==0 && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==3) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-3]==1 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-3]==3)){
							return 0;
						}
						//double check up right
						else if((pieces[rowLoc][0]+4>=0 && pieces[rowLoc][1]+4<8) && boxes[pieces[rowLoc][0]+4][pieces[rowLoc][1]+4]==0 && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==3) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+3]==1 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+3]==3)){
							return 0;
						}
						//double check same column up
						else if(pieces[rowLoc][0]+4>=0 && boxes[pieces[rowLoc][0]+4][pieces[rowLoc][1]]==0){
							//same column from right
							if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==3) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]+1]==3)){
								return 0;
							}
							//same column from right
							if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==3) && (boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]+3][pieces[rowLoc][1]-1]==3)){
								return 0;
							}
					}	
				}
		/*KING moves*/
		if(pieces[rowLoc][2]==1){
			//double check up left
			if((pieces[rowLoc][0]-4>=0 && pieces[rowLoc][1]-4>=0) && boxes[pieces[rowLoc][0]-4][pieces[rowLoc][1]-4]==0 && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==3) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-3]==1 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-3]==3)){
				return 0;
			}
			//double check up right
			else if((pieces[rowLoc][0]-4>=0 && pieces[rowLoc][1]+4<8) && boxes[pieces[rowLoc][0]-4][pieces[rowLoc][1]+4]==0 && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==3) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+3]==1 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+3]==3)){			
				return 0;
			}
		}
		//double check same column up
		if(pieces[rowLoc][0]-4>=0 && boxes[pieces[rowLoc][0]-4][pieces[rowLoc][1]]==0){
			//same column from right
			if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==3) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]+1]==3)){
				return 0;
			}
			//same column from left
			if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==3) && (boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]-3][pieces[rowLoc][1]-1]==3)){
				return 0;					
			}
		}
		//double check same row
			//to left
			if(boxes[pieces[rowLoc][0]][pieces[rowLoc][1]-4]==0){
				//double check left from up
				if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==3) && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-3]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-3]==3)){
					return 0;
				}
				//double check left from down
				if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==3) && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-3]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-3]==3)){
						return 0;
				}
			}
			//to right
			if(boxes[pieces[rowLoc][0]][pieces[rowLoc][1]+4]==0){
				//double check right from up
				if(boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==3) && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+3]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+3]==3)){
					return 0;
				}
				//double check right from down
				if(boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==3) && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+3]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+3]==3)){
						return 0;
				}
			}
			rowLoc++;
		}
		

		//computer looking for a single check
		rowLoc=0;
			while(!(pieces[rowLoc][0]==0 && pieces[rowLoc][1]==0) && rowLoc<12){
					/*KING MOVES*/
					if(pieces[rowLoc][2]==1){
						//single check up left
						if((pieces[rowLoc][0]-2>=0 && pieces[rowLoc][1]-2>=0) && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==3)){
							return 0;
						}
						//single check up right
						else if((pieces[rowLoc][0]-2>=0 && pieces[rowLoc][1]+2<8) && boxes[pieces[rowLoc][0]-2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==3)){
							return 0;
						}
						/*Simple moves*/
					if(pieces[rowLoc][2]==1 || pieces[rowLoc][2]==0){
						//single check down left
						if((pieces[rowLoc][0]+2<8 && pieces[rowLoc][1]-2>=0) && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]-2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==3)){
							return 0;
						}
						//single check down right
						else if((pieces[rowLoc][0]+2<8 && pieces[rowLoc][1]+2<8) && boxes[pieces[rowLoc][0]+2][pieces[rowLoc][1]+2]==0 && (boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==1 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==3)){
							return 0;
						}	
					}
				}		
			rowLoc++;
			}
			rowLoc=0;
			while(!(pieces[rowLoc][0]==0 && pieces[rowLoc][1]==0) && rowLoc<12){
				/*King Moves*/
				if(pieces[rowLoc][2]==1 || pieces[rowLoc][2]==0){
					//moving up
					if(boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]+1]==0 || boxes[pieces[rowLoc][0]-1][pieces[rowLoc][1]-1]==0){
						return 0;
					}
				}
				/*simple piece moves*/
				if(pieces[rowLoc][2]==1){
					//moving down
					if(boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]+1]==0 || boxes[pieces[rowLoc][0]+1][pieces[rowLoc][1]-1]==0){
						return 0;
					}
				}
			rowLoc++;
			}
			return 1;//tava comentado
	}
	return 1;
}

//function definition move store
void moveStore(uint8_t turn, char row, uint8_t col,char newRow, uint8_t newCol){	
	printf("\nMOVESTORE\n");

		RankJogadas[num_play].Xinit = row;
		RankJogadas[num_play].Yinit = col;
		RankJogadas[num_play].Xfinal = newRow;
		RankJogadas[num_play++].Yfinal = newCol;

		if(turn==1){
			printf("%c%d %c%d\n", row, col, newRow, newCol);           //print moves of player 1
		}
		else if(turn==2){
			printf("%c%d %c%d\n", row, col, newRow, newCol);           //print moves of player 2
		}
}

/*function definition welcome screen*/
void welcome(){
	printf("Welcome\n");
	printf("\n\n\n");
}

//definition of player turn
uint8_t playermoves(uint8_t player, uint8_t row, uint8_t col, uint8_t newRow, uint8_t newCol,char oldRowChar, char newRowChar, uint8_t *valid){
	//uint8_t valid=0;										//flag value for move validity 
	//ja esta preparada para os dois jogadores

	//Se o sitio para onde esta a mover for invalido entao pede para fazer movimento outra vez
		if((boxes[row][col] == -1)||(boxes[newRow][newCol] == -1)) 
			playerTurn(player);

		if((boxes[row][col] == 0)||(boxes[newRow][newCol] != 0))
			playerTurn(player);

		printf("\n player: %d moves\n",player);
			//for player simple piece
			if(boxes[row][col]== player && boxes[newRow][newCol]==0){					
					*valid = (player == 1? moveDown(boxes[row][col],row,col,newRow,newCol):
										  moveUp(boxes[row][col],row,col,newRow,newCol));	//player call move down function
					if(*valid==1){
						checkKing();						//checking for king
						print_board(boxes);					//refreshing the board
						if(countPieces((player == 1 ? 2:1))==0){				//checking if player has won
							win(player);
						}
						else{
							moveStore(player,oldRowChar,col+1,newRowChar,newCol+1);	
							playerTurn((player == 1) ? 2:1);
						}	
					}
					else playerTurn(player);
			}
			//for player  king piece ( p1 = 3; p2 = 4)
			else if(boxes[row][col]==(player + 2)){
					//if moving down
					if(newRow>row){
							*valid = moveDown(boxes[row][col],row,col,newRow,newCol);
							if(*valid==1){
								checkKing();						//checking for king
								print_board(boxes);					//refreshing the board
								if(countPieces((player == 1 ? 2:1))==0){
									win(player);
								}
								else{
									moveStore(player,oldRowChar,col+1,newRowChar,newCol+1);
									playerTurn((player == 1) ? 2:1);
								}	
							}
							else playerTurn(player);
					}
					//if moving up
					else if(newRow<row){
							*valid = moveUp(boxes[row][col],row,col,newRow,newCol);
							if(*valid==1){
								checkKing();						//checking for king
								print_board(boxes);					//refreshing the board
								if(countPieces((player == 1 ? 2:1))==0){
									win(player);
								}
								else{
									moveStore(player,oldRowChar,col+1,newRowChar,newCol+1);
									playerTurn((player == 1) ? 2:1);
								}	
							}
							else playerTurn(player);
					}	
					else if(newRow==row){ // movimento diagonal cima, seguido de diagonal baixo para o mesmo lado \/  ?
							*valid = backFor(boxes[row][col],row,col,newRow,newCol);
							if(*valid==1){
								checkKing();						//checking for queen
								print_board(boxes);					//refreshing the board
								if(countPieces((player == 1 ? 2:1))==0){
									win(player);
								}
								else{
									moveStore(player,oldRowChar,col+1,newRowChar,newCol+1);
									playerTurn((player == 1) ? 2:1);
								}	
							}
							else{
								// printf("\n teste\n");
								playerTurn(player);
							}
					}
				}
			else{
				printf("\njoga \n");
				playerTurn(player);
			}
	printf("Invalido\n");
	playerTurn(player);
	return 1;
}	

void playerTurn(uint8_t player){

	uint8_t valid=0;														//flag value for move validity 
	uint8_t row = 0, col = 0, newRow = 0, newCol = 0;						//for storing the moves entered by player
	char oldRowChar,newRowChar;
	uint8_t moveSelected;

	if(player == 1){
		PORTB |= (1 << PB0);
		PORTD &= ~(1 << PD7);
	}
	else{
		PORTD |= (1 << PD7);
		PORTB &= ~(1 << PB0);
	}


	comeCome(pecaRetirar,&posPecaRetirar);

	while(valid!=1){

		if(player==1){
				int Moves;
				Moves = noMoves(1);
				if(Moves==1)
					win(2);
		}
		else if(player==2){
				int Moves;
				Moves = noMoves(2);
				if(Moves==1){
					win(1);
			}		
		}


		_delay_ms(3000);
		printf("\nplayer %d selecione a sua jogada\n", player);

		moveSelected = play(&row, &col, &newRow, &newCol);

		if(moveSelected){ 
			oldRowChar = convNum2Char(row);
			newRowChar = convNum2Char(newRow);

			playermoves(player,row,col,newRow,newCol,oldRowChar,newRowChar,&valid);
			// move_servo_AtoB(row,col,newRow,newCol);
			row = 0;
			col = 0;
			newRow = 0;
			newCol = 0;
		}
	}
}

int main()
{		
 	io_init();
	init_mili_timers(1);
  	servo_tc1_PWM_init();
  	servo_tc0_PWM_init();
	printf_init();
	buzzer_init();

	welcome();                                              //printing the welcome screen
	calculo_pos();
	// unsigned long timer_freq;
	enum t2_prescaler ps = T2_PRESCALER_256;
	t2_set_prescaler(ps);
	// timer_freq = div_round(F_CPU, t2_get_prescaler_rate(ps));

	DDRC &= ~(1<<PC0);

	while(jogar){
			print_board(boxes);										//printing the game board
			playerTurn(1);											//set first turn to player1

	}
	/*	
		while(1){
			printf("Teste situacao vencedora: player 1 ganha e depois player 2 ganha\n");
			_delay_us(5000);
			win(1);
			_delay_ms(1000);
			win(2);
		}
	*/

	/*
		int i;
		printf("Teste EEPROM: Registo do historico das jogadas do jogo mais rapido\n");
		printf("Jogo 1:\n");

		for (int i = 0; i < 3; i++)
		{
			RankJogadas[i].Xinit = convNum2Char(i+1);
			RankJogadas[i].Yinit = i*2;
			RankJogadas[i].Xfinal = convNum2Char(i*2);
			RankJogadas[i].Yfinal = i;

			num_play++;
		}
		printf("fim jogo 1\n");
		win(1);
		_delay_ms(2000);
		num_play = 0;
		printf("jogo 2:\n");
		for (i = 0; i < 2; i++)
		{
			RankJogadas[i].Xinit = convNum2Char(i*3+1);
			RankJogadas[i].Yinit = i;
			RankJogadas[i].Xfinal = convNum2Char(i);
			RankJogadas[i].Yfinal = i*3;
			num_play ++;
		}
		printf("fim jogo 2\n");
		win(2);
		_delay_ms(2000);

		num_play = 0;
		printf("jogo 3:\n");
		for ( i = 0; i < 5; i++)
		{
			RankJogadas[i].Xinit = convNum2Char(i+1);
			RankJogadas[i].Yinit = i+1;
			RankJogadas[i].Xfinal = convNum2Char(i+1);
			RankJogadas[i].Yfinal = i+1;
			num_play ++;
		}
		printf("fim jogo\n");
		win(1);
	*/
	
	


}
