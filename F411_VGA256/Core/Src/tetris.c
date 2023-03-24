/*
 * tetris.c
 *
 *  Created on: Feb 24, 2023
 *      Author: thanwa
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tetris.h"
#include "vga256.h"

#define SIZE 	3
#define HEIGHT  30
#define WIDTH 	13
#define V_BGND	0		//Background color
#define V_BORD  VGA_WHITE //Border color
#define BOARD_X 1
#define BOARD_Y 1
#define UPSPEED 100
#define NUMLEVEL 10

int tetrisBoard[HEIGHT][WIDTH];

// 7 types of pieces
const int8_t piece[7][4][8]=
{
		{ //Straight tetromino
				{0,-1,0,0,0,1,0,2},
				{-1,0,0,0,1,0,2,0},
				{0,-1,0,0,0,1,0,2},
				{-1,0,0,0,1,0,2,0}
		},{//Invert L tetromino
				{-1,1,0,1,0,0,0,-1},
				{-1,-1,-1,0,0,0,1,0},
				{1,-1,0,-1,0,0,0,1},
				{1,1,1,0,0,0,-1,0}
		},{//L tetromino
				{1,1,0,1,0,0,0,-1},
				{-1,1,-1,0,0,0,1,0},
				{-1,-1,0,-1,0,0,0,1},
				{-1,0,0,0,1,0,1,-1}
		},{//Square tetromino
				{0,0,1,0,1,1,0,1},
				{0,0,1,0,1,1,0,1},
				{0,0,1,0,1,1,0,1},
				{0,0,1,0,1,1,0,1}
		},{//Skew-right tetromino
				{-1,0,0,0,0,-1,1,-1},
				{0,-1,0,0,1,0,1,1},
				{-1,0,0,0,0,-1,1,-1},
				{0,-1,0,0,1,0,1,1}
		},{//T tetromino
				{-1,0,0,0,1,0,0,1},
				{0,-1,0,0,0,1,-1,0},
				{-1,0,0,0,1,0,0,-1},
				{0,-1,0,0,0,1,1,0}
		},{//Skew-left tetromino
				{-1,0,0,0,0,1,1,1},
				{0,0,1,0,1,-1,0,1},
				{-1,0,0,0,0,1,1,1},
				{0,0,1,0,1,-1,0,1}
		}
};

const uint8_t pieceColor[7]={VGA_CYAN,	//long piece
							VGA_BLUE,	//inverted L
							VGA_ORANGE,	//L
							VGA_YELLOW, //square
							VGA_GREEN,  //skew right
							VGA_PURPLE, //T
							VGA_RED};   //skew left

int score;
int fallDelay,curDelay;

int curX,curY,curR,curPiece;
int running;
int Round=0;

//These functions declared elsewhere in the project
extern void tetrisDelay(int ms);
extern uint8_t getch(uint8_t *ch);
extern uint32_t tetrisSeed();

void showScore();
int  rotateAble(int pieceNum,int pieceTargetRotation,int locX,int locY);
int  moveAble(int pieceNum,int pieceRotation,int locX,int locY);
int  pieceOverlapped();
void drawPiece(int pieceNum,int pieceRotation,int locX,int locY,int color);
void drawBoard();
void checkRows();


void tetris(){
	int x,y;
	uint8_t ch;
	//Initial values
	srand(tetrisSeed());
	fallDelay = NUMLEVEL;
	curDelay=0;
	curX = WIDTH/2;
	curY = 0;
	curR = 0;
	curPiece = rand()%7;
	score=0;
	running = 1;
    //Initialize board
	for(y=0;y<HEIGHT;y++){
		for(x=0;x<WIDTH;x++){
			tetrisBoard[y][x]=V_BGND;
		}
	}
	//Clear screen and draw borders
	ClearScreen(V_BGND);
	FillRectangle(BOARD_X,BOARD_Y,
			      BOARD_X+SIZE-1,BOARD_Y+(HEIGHT+2)*SIZE-1,V_BORD);
	FillRectangle(BOARD_X+(WIDTH+1)*SIZE,BOARD_Y,
			      BOARD_X+(WIDTH+2)*SIZE-1,BOARD_Y+(HEIGHT+2)*SIZE-1,V_BORD);
	FillRectangle(BOARD_X+SIZE,BOARD_Y,
			      BOARD_X+(WIDTH+1)*SIZE-1,BOARD_Y+SIZE-1,V_BORD);
	FillRectangle(BOARD_X+SIZE,BOARD_Y+(HEIGHT+1)*SIZE,
			      BOARD_X+(WIDTH+1)*SIZE-1,
				  BOARD_Y+(HEIGHT+2)*SIZE-1,V_BORD);

	//Main Game Loop
	while(running){
		//Checking for key pressed and move current piece
		if(getch(&ch)){
			switch(ch){
			case '0': // Exit Tetris
				running = 0;
				continue;
			case '8': // rotate
				if(rotateAble(curPiece,(curR+1)%4,curX,curY)){
					//remove old piece
					drawPiece(curPiece,curR,curX,curY,V_BGND);
					//rotate piece and re-draw
					curR = (curR+1)%4;
					drawPiece(curPiece,curR,curX,curY,pieceColor[curPiece]);
			    }
				break;
			case '4': // move left
				if(rotateAble(curPiece,curR,curX-1,curY)){
				    //remove old piece
					drawPiece(curPiece,curR,curX,curY,V_BGND);
					//rotate piece and re-draw
					curX = curX-1;
					drawPiece(curPiece,curR,curX,curY,pieceColor[curPiece]);
				}
				break;
			case '6': // move right
				if(rotateAble(curPiece,curR,curX+1,curY)){
				    //remove old piece
					drawPiece(curPiece,curR,curX,curY,V_BGND);
					//rotate piece and re-draw
					curX = curX+1;
					drawPiece(curPiece,curR,curX,curY,pieceColor[curPiece]);
				}
				break;
			case '2': // move down
				if(rotateAble(curPiece,curR,curX,curY+1)){
					//remove old piece
					drawPiece(curPiece,curR,curX,curY,V_BGND);
					//rotate piece and re-draw
					curY = curY+1;
					drawPiece(curPiece,curR,curX,curY,pieceColor[curPiece]);
				}
				break;
		}
		}
		curDelay--;
		if(curDelay<0){
			Round++;
			if(Round>UPSPEED){
				Round = 0;
				if(fallDelay>0)
					fallDelay--;
			}
			curDelay=fallDelay;
		    // Check if current piece can move further down
		    if(moveAble(curPiece,curR,curX,curY)){
			    //remove old piece
		    	drawPiece(curPiece,curR,curX,curY,V_BGND);
		    	//rotate piece and re-draw
		    	curY = curY+1;
		    	drawPiece(curPiece,curR,curX,curY,pieceColor[curPiece]);
		    }else{
		    	// struck here
		    	//Assign value to the board
		    	for(int i=0;i<4;i++){
		    		tetrisBoard[curY+piece[curPiece][curR][i*2+1]]
							   [curX+piece[curPiece][curR][i*2]] = pieceColor[curPiece];
		        }
		    	score = score+4; // Add score of placing new piece in the board

		    	//Check for completed rows
		    	checkRows();
		    	showScore();
			    //Generate new piece
			    curPiece = rand()%7;
			    curX = WIDTH/2;
			    curY = 0;
			    curR = 0;

			    if(pieceOverlapped()){
			    	// Can not place new piece
			    	running = 0;
			    	continue;
			    }
		    }
		}
		tetrisDelay(100); //Delay one unit
	}
}


void showScore(){
	char msg[16];
	SetCursor((BOARD_X+WIDTH+2)*SIZE,BOARD_Y+SIZE);
	WriteString(" SCORE",Font_7x10,VGA_CYAN);
	sprintf(msg,"%6d",score);
	SetCursor((BOARD_X+WIDTH+2)*SIZE,BOARD_Y+SIZE+12);
	WriteString(msg,Font_7x10,VGA_YELLOW);
	SetCursor((BOARD_X+WIDTH+2)*SIZE,BOARD_Y+SIZE+24);
	WriteString(" LEVEL",Font_7x10,VGA_CYAN);
	sprintf(msg,"%6d",NUMLEVEL-fallDelay+1);
	SetCursor((BOARD_X+WIDTH+2)*SIZE,BOARD_Y+SIZE+36);
	WriteString(msg,Font_7x10,VGA_GREEN);
}
int  rotateAble(int pieceNum,int pieceTargetRotation,int locX,int locY){
    for(int i=0;i<4;i++){
    	int x = piece[pieceNum][pieceTargetRotation][i*2]+locX;
    	int y = piece[pieceNum][pieceTargetRotation][i*2+1]+locY;
    	if(x<0) return 0; // Out of board
    	if((x>=WIDTH) || (y>=HEIGHT)) return 0; // Out of board
    	if(tetrisBoard[y][x]!=V_BGND) return 0; // Overlapped other pieces
    }
    return 1;
}

int  moveAble(int pieceNum,int pieceRotation,int locX,int locY){
	for(int i=0;i<4;i++){
    	int x = piece[pieceNum][pieceRotation][i*2]+locX;
    	int y = piece[pieceNum][pieceRotation][i*2+1]+locY;
    	//check if it is at the buttom
    	if(y>=(HEIGHT-1)) return 0;
    	//check if there is another piece under the current one
    	if(tetrisBoard[y+1][x]!=V_BGND)
    		return 0;
	}
	return 1;
}

int  pieceOverlapped(){
	for(int i=0;i<4;i++){
    	int x = piece[curPiece][curR][i*2]+curX;
    	int y = piece[curPiece][curR][i*2+1]+curY;
    	//check if there is another piece under the current one
    	if(y<0) continue;
    	if(tetrisBoard[y+1][x]!=V_BGND)
    		return 1;
	}
	return 0;
}

void  drawPiece(int pieceNum,int pieceRotation,int locX,int locY,int color){
	for(int i=0;i<4;i++){
    	int x = piece[pieceNum][pieceRotation][i*2]+locX;
    	int y = piece[pieceNum][pieceRotation][i*2+1]+locY;
    	if(y>=0){
    	    FillRectangle(BOARD_X+(x+1)*SIZE,BOARD_Y+(y+1)*SIZE,
    	    		      BOARD_X+(x+2)*SIZE-1,BOARD_Y+(y+2)*SIZE-1,color);
    	}
	}
}

void drawBoard(){
	for(int y=0;y<HEIGHT;y++){
		for(int x=0;x<WIDTH;x++){
			FillRectangle(BOARD_X+(x+1)*SIZE,BOARD_Y+(y+1)*SIZE,
					      BOARD_X+(x+2)*SIZE-1,BOARD_Y+(y+2)*SIZE-1,
					       tetrisBoard[y][x]);
		}
	}
}

void checkRows(){
	int addedScore = 100;
	int y,x,yy;
	int flag;

	for(y=HEIGHT-1;y>=0;y--){
		flag = 1;
		//check current row if all filled
		for(x=0;x<WIDTH;x++){
			if(tetrisBoard[y][x]==V_BGND){
				flag = 0;
				break;
			}
		}
		if(flag){
			score += addedScore;  //increase score
			addedScore *=2;
			//Bunch pieces down
			for(yy=y-1;yy>=0;yy--){
			   for(x=0;x<WIDTH;x++){
				   tetrisBoard[yy+1][x] = tetrisBoard[yy][x];
			   }
			}
			for(x=0;x<WIDTH;x++){
				   tetrisBoard[0][x] = V_BGND;
			}
			y = HEIGHT; // recheck from bottom
			drawBoard();
			tetrisDelay(200);
		}
	}
}
