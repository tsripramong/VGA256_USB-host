/**
 * original author:  Tilen Majerle<tilen@majerle.eu>
 * modification for STM32f10x: Alexander Lutsai<s.lyra@ya.ru>
   ----------------------------------------------------------------------
   	Copyright (C) Alexander Lutsai, 2016
    Copyright (C) Tilen Majerle, 2015
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
     
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
 */
#include <stdint.h>

#ifndef __VGAFONTS_H__
#define __VGAFONTS_H__

typedef struct {
	const uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef;

#define VGA_INCLUDE_FONT_6x8
#define VGA_INCLUDE_FONT_7x10
#define VGA_INCLUDE_FONT_11x18
#define VGA_INCLUDE_FONT_16x26


#ifdef VGA_INCLUDE_FONT_6x8
extern FontDef Font_6x8;
#endif
#ifdef VGA_INCLUDE_FONT_7x10
extern FontDef Font_7x10;
#endif
#ifdef VGA_INCLUDE_FONT_11x18
extern FontDef Font_11x18;
#endif
#ifdef VGA_INCLUDE_FONT_16x26
extern FontDef Font_16x26;
#endif
#endif // __VGAFONTS_H__
