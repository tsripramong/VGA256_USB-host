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
