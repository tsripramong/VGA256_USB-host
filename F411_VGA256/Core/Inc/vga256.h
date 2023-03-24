/*
 * vga256.h
 *
 *  Created on: Feb 16, 2023
 *      Author: thanwa
 */
#include <stdint.h>
#include "vgafonts.h"

#ifndef INC_VGA256_H_
#define INC_VGA256_H_

#define VGA_WIDTH   160		//Displayable area (Width)
#define VGA_HEIGHT  116		//Displayable area (Height)

#define VGA_LBUFFER 160		//Local Video RAM buffer (Width)
#define VGA_VBUFFER 120		//Local Video RAM buffer (Height)

#define VGA_LBUFFERSIZE 200	//Circular DMA RAM bufer (1-line)
#define VGA_FULL	1600
#define VGA_HALF	800

#define VGA_WHITE	0xff
#define VGA_BLACK	0x00
#define VGA_RED		0xe0
#define VGA_GREEN	0x1c
#define VGA_BLUE	0x03
#define VGA_GRAY	0x49
#define VGA_YELLOW	0xfc
#define VGA_ORANGE	0xec
#define VGA_CYAN	0x1f
#define VGA_PURPLE  0xe3
#define VGA_COLOR uint8_t
#define VGA_Color(r,g,b)   (((uint8_t)r&0xe0)|(((uint8_t)g>>3)&0x1c)|(((uint8_t)b>>6)&0x03))

extern uint8_t VGA_obuffer[VGA_FULL];
extern uint8_t VGA_buffer[VGA_VBUFFER][VGA_LBUFFER];
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
} VGA_t;

void ClearScreen(VGA_COLOR color);
void DrawPixel(int16_t x, int16_t y, VGA_COLOR color);
void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, VGA_COLOR c);
char WriteChar(char ch, FontDef Font, VGA_COLOR color);
char WriteString(char* str, FontDef Font, VGA_COLOR color);
void SetCursor(int16_t x, int16_t y);
void DrawArc(int16_t x, int16_t y, int16_t radius, int16_t start_angle, int16_t sweep, VGA_COLOR color);
void DrawCircle(int16_t par_x,int16_t par_y,int16_t par_r,VGA_COLOR par_color);
void DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, VGA_COLOR color);
void FillRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, VGA_COLOR color);
void FillCircle(int16_t x0, int16_t y0, int16_t r, VGA_COLOR c);
void ShowImage(uint8_t *image,int16_t x,int16_t y,int16_t locX,int16_t locY);
#endif /* INC_VGA256_H_ */
