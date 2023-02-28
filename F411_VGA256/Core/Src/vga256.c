/*
 * vgamono.c
 *
 *  Created on: Feb 16, 2023
 *      Author: thanwa
 */

#include "vga256.h"
#include "math.h"

uint8_t VGA_obuffer[VGA_FULL];
uint8_t VGA_buffer[VGA_VBUFFER][VGA_LBUFFER];
VGA_t VGA;

void ClearScreen(VGA_COLOR color){
	int i,j;
	for(j=0;j<VGA_VBUFFER;j++)
		for(i=0;i<VGA_LBUFFER;i++){
			VGA_buffer[j][i]= color;
	}
}

void DrawPixel(int16_t x, int16_t y, VGA_COLOR color) {
	if ((x <0) || (y <0)||
	    (x >= VGA_WIDTH) || (y >= VGA_HEIGHT)) {
		/* Error */
		return;
	}
	/* Set color */
	VGA_buffer[y][x]=color;
}

void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, VGA_COLOR c) {
	int16_t dx, dy, sx, sy, err, e2, i, tmp;

	/* Check for overflow */
/*	if (x0 >= VGA_WIDTH) {
		x0 = VGA_WIDTH - 1;
	}
	if (x1 >= VGA_WIDTH) {
		x1 = VGA_WIDTH - 1;
	}
	if (y0 >= VGA_HEIGHT) {
		y0 = VGA_HEIGHT - 1;
	}
	if (y1 >= VGA_HEIGHT) {
		y1 = VGA_HEIGHT - 1;
	}
*/
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			DrawPixel(x0, i, c);
		}

		/* Return from function */
		return;
	}

	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}

		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}

		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			DrawPixel(i, y0, c);
		}

		/* Return from function */
		return;
	}

	while (1) {
		DrawPixel(x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

char WriteChar(char ch, FontDef Font, VGA_COLOR color) {
    uint32_t i, b, j;

    // Check if character is valid
    if (ch < 32 || ch > 126)
        return 0;

    // Check remaining space on current line
    if (VGA_WIDTH < (VGA.CurrentX + Font.FontWidth) ||
        VGA_HEIGHT < (VGA.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for(i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for(j = 0; j < Font.FontWidth; j++) {
            if((b << j) & 0x8000)  {
                DrawPixel(VGA.CurrentX + j, (VGA.CurrentY + i), (VGA_COLOR) color);
            } else {
                DrawPixel(VGA.CurrentX + j, (VGA.CurrentY + i), (VGA_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    VGA.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

char WriteString(char* str, FontDef Font, VGA_COLOR color) {
    // Write until null-byte
    while (*str) {
        if (WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

// Position the cursor
void SetCursor(int16_t x, int16_t y) {
	if((x<0)||(x>=VGA_WIDTH))
	    VGA.CurrentX = 0;
	else
		VGA.CurrentX = x;
	if((y<0)||(y>=VGA_HEIGHT))
	    VGA.CurrentY = 0;
	else
        VGA.CurrentY = y;
}

/*Convert Degrees to Radians*/
static float VGA_DegToRad(float par_deg) {
    return par_deg * 3.14 / 180.0;
}

/*Normalize degree to [0;360]*/
static uint16_t VGA_NormalizeTo0_360(uint16_t par_deg) {
  uint16_t loc_angle;
  if(par_deg <= 360)
  {
    loc_angle = par_deg;
  }
  else
  {
    loc_angle = par_deg % 360;
    loc_angle = ((par_deg != 0)?par_deg:360);
  }
  return loc_angle;
}

void DrawArc(int16_t x, int16_t y, int16_t radius, int16_t start_angle, int16_t sweep, VGA_COLOR color) {
    #define CIRCLE_APPROXIMATION_SEGMENTS 36
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1,xp2;
    uint8_t yp1,yp2;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;

    loc_sweep = VGA_NormalizeTo0_360(sweep);

    count = (VGA_NormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;
    while(count < approx_segments)
    {
        rad = VGA_DegToRad(count*approx_degree);
        xp1 = x + (int16_t)(sin(rad)*radius);
        yp1 = y + (int16_t)(cos(rad)*radius);
        count++;
        if(count != approx_segments)
        {
            rad = VGA_DegToRad(count*approx_degree);
        }
        else
        {
            rad = VGA_DegToRad(loc_sweep);
        }
        xp2 = x + (int16_t)(sin(rad)*radius);
        yp2 = y + (int16_t)(cos(rad)*radius);
        DrawLine(xp1,yp1,xp2,yp2,color);
    }

    return;
}

void DrawCircle(int16_t par_x,int16_t par_y,int16_t par_r,VGA_COLOR par_color) {
  int32_t x = -par_r;
  int32_t y = 0;
  int32_t err = 2 - 2 * par_r;
  int32_t e2;

  if (par_x >= VGA_WIDTH || par_y >= VGA_HEIGHT) {
    return;
  }

    do {
      DrawPixel(par_x - x, par_y + y, par_color);
      DrawPixel(par_x + x, par_y + y, par_color);
      DrawPixel(par_x + x, par_y - y, par_color);
      DrawPixel(par_x - x, par_y - y, par_color);
        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if(-x == y && e2 <= x) {
              e2 = 0;
            }
            else
            {
              /*nothing to do*/
            }
        }
        else
        {
          /*nothing to do*/
        }
        if(e2 > x) {
          x++;
          err = err + (x * 2 + 1);
        }
        else
        {
          /*nothing to do*/
        }
    } while(x <= 0);

    return;
}

void DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, VGA_COLOR color) {
  DrawLine(x1,y1,x2,y1,color);
  DrawLine(x2,y1,x2,y2,color);
  DrawLine(x2,y2,x1,y2,color);
  DrawLine(x1,y2,x1,y1,color);
  return;
}

void FillRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, VGA_COLOR color) {
    // clipping
    if((x1 >= VGA_WIDTH) || (y1 >= VGA_HEIGHT)) return;
    if(x2 >= VGA_WIDTH) x2 = VGA_WIDTH-1;
    if(y2 >= VGA_HEIGHT) y2 = VGA_HEIGHT-1;
    for(int yy = y1; yy <= y2; yy++) {
    	DrawLine(x1,yy,x2,yy,color);
    }
}

void FillCircle(int16_t x0, int16_t y0, int16_t r, VGA_COLOR c) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    DrawPixel(x0, y0 + r, c);
    DrawPixel(x0, y0 - r, c);
    DrawPixel(x0 + r, y0, c);
    DrawPixel(x0 - r, y0, c);
    DrawLine(x0 - r, y0, x0 + r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
        DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);

        DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
        DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}

void ShowImage(uint8_t *image,int16_t x,int16_t y,int16_t locX,int16_t locY){
    int16_t j,i;

    for(j=0;j<y;j++){
    	for(i=0;i<x;i++){
    		DrawPixel(i+locX,j+locY,image[j*x+i]);
    	}
    }
}
