/*
Sea Base Omega - C Section
Copyright (C) 2013, Ben "GreaseMonkey" Russell & contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "common.h"

/**
	\brief Blit an image from one raw 32bpp surface to another.
	
	\param sd Source data.
	\param sw Source width in pixels.
	\param sp Source pitch in bytes.
	\param sh Source height.
	\param sx Source X coordinate.
	\param sy Source Y coordinate.
	\param dd Destination data.
	\param dw Destination width in pixels.
	\param dp Destination pitch in bytes.
	\param dh Destination height.
	\param dx Destination X coordinate.
	\param dy Destination Y coordinate.
	\param bw Blit width.
	\param bh Blit height.
	\param flags Blit flags (BF_*).
*/
void blit_raw(
	uint32_t *sd, int sw, int sp, int sh, int sx, int sy,
	uint32_t *dd, int dw, int dp, int dh, int dx, int dy,
	int bw, int bh, int flags)
{
	// Clip Top-Left
	if(sx < 0) { bw += sx; dx -= sx; sx -= sx; }
	if(sy < 0) { bh += sy; dy -= sy; sy -= sy; }
	if(dx < 0) { bw += dx; sx -= dx; dx -= dx; }
	if(dy < 0) { bh += dy; sy -= dy; dy -= dy; }

	// Clip Bottom-Right
	if(sx + bw - sw > 0) { bw -= sx + bw - sw; }
	if(sy + bh - sh > 0) { bh -= sy + bh - sh; }
	if(dx + bw - dw > 0) { bw -= dx + bw - dw; }
	if(dy + bh - dh > 0) { bh -= dy + bh - dh; }

	//printf("raw blit %i %i %i %i %i %i\n", sx, sy, dx, dy, bw, bh);

	// Eliminate empty blits
	if(bw <= 0 || bh <= 0)
		return;

	// Find top-left corner
	uint8_t *sd_b = (uint8_t *)sd;
	uint8_t *dd_b = (uint8_t *)dd;
	sd = sx + (uint32_t *)(sd_b + sp*sy);
	dd = dx + (uint32_t *)(dd_b + dp*dy);
	sd_b = (uint8_t *)sd;
	dd_b = (uint8_t *)dd;

	// Blit
	int x, y;

	switch(flags & BF_M_AMODE)
	{
		case BF_AM_DIRECT:
			for(y = 0; y < bh; y++)
			{
				memcpy(dd_b, sd_b, bw*4);
				sd_b += sp;
				dd_b += dp;
			}
			break;
		case BF_AM_THRES:
			for(y = 0; y < bh; y++)
			{
				sd = (uint32_t *)sd_b;
				dd = (uint32_t *)dd_b;

				for(x = 0; x < bw; x++, sd++, dd++)
					if(*sd >= 0x80000000U)
						*dd = *sd;

				sd_b += sp;
				dd_b += dp;
			}
			break;
		default:
			eprintf("PANIC: blit mode %i not supported!\n",
				(flags & BF_M_AMODE));
			fflush(stderr);
			abort();
	}
}

/**
	\brief Blit an image from an img_t to an img_t.
	
	\param sd Source image.
	\param sx Source X coordinate.
	\param sy Source Y coordinate.
	\param dd Destination image.
	\param dx Destination X coordinate.
	\param dy Destination Y coordinate.
	\param bw Blit width.
	\param bh Blit height.
	\param flags Blit flags (BF_*).
*/
void blit_img_to_img(
	img_t *sd, int sx, int sy,
	img_t *dd, int dx, int dy,
	int bw, int bh, int flags)
{
	return blit_raw(
		sd->data, sd->w, sd->w*4, sd->h, sx, sy,
		dd->data, dd->w, dd->w*4, dd->h, dx, dy,
		bw, bh, flags);
}

/**
	\brief Blit an image from an img_t to an SDL_Surface.
	
	\param sd Source image.
	\param sx Source X coordinate.
	\param sy Source Y coordinate.
	\param dd Destination image.
	\param dx Destination X coordinate.
	\param dy Destination Y coordinate.
	\param bw Blit width.
	\param bh Blit height.
	\param flags Blit flags (BF_*).
*/
void blit_img_to_sdl(
	img_t *sd, int sx, int sy,
	SDL_Surface *dd, int dx, int dy,
	int bw, int bh, int flags)
{
	SDL_LockSurface(dd);
	blit_raw(
		sd->data, sd->w, sd->w*4, sd->h, sx, sy,
		(uint32_t *)(dd->pixels), dd->w, dd->pitch, dd->h, dx, dy,
		bw, bh, flags);
	SDL_UnlockSurface(dd);
}

/**
	\brief Blit an image from an SDL_Surface to an img_t.
	
	\param sd Source image.
	\param sx Source X coordinate.
	\param sy Source Y coordinate.
	\param dd Destination image.
	\param dx Destination X coordinate.
	\param dy Destination Y coordinate.
	\param bw Blit width.
	\param bh Blit height.
	\param flags Blit flags (BF_*).
*/
void blit_sdl_to_img(
	SDL_Surface *sd, int sx, int sy,
	img_t *dd, int dx, int dy,
	int bw, int bh, int flags)
{
	SDL_LockSurface(sd);
	blit_raw(
		(uint32_t *)(sd->pixels), sd->w, sd->pitch, sd->h, sx, sy,
		dd->data, dd->w, dd->w*4, dd->h, dx, dy,
		bw, bh, flags);
	SDL_UnlockSurface(sd);
}

/**
	\brief Blit an image from an SDL_Surface to an SDL_Surface.
	
	\param sd Source image.
	\param sx Source X coordinate.
	\param sy Source Y coordinate.
	\param dd Destination image.
	\param dx Destination X coordinate.
	\param dy Destination Y coordinate.
	\param bw Blit width.
	\param bh Blit height.
	\param flags Blit flags (BF_*).
*/
void blit_sdl_to_sdl(
	SDL_Surface *sd, int sx, int sy,
	SDL_Surface *dd, int dx, int dy,
	int bw, int bh, int flags)
{
	SDL_LockSurface(sd);
	SDL_LockSurface(dd);
	blit_raw(
		(uint32_t *)(sd->pixels), sd->w, sd->pitch, sd->h, sx, sy,
		(uint32_t *)(dd->pixels), dd->w, dd->pitch, dd->h, dx, dy,
		bw, bh, flags);
	SDL_UnlockSurface(dd);
	SDL_UnlockSurface(sd);
}

