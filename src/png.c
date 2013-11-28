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

/*
This file is also available under the zlib licence:

  Copyright (C) 2013 Ben "GreaseMonkey" Russell & contributors

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "common.h"

/**
	\brief Loads a PNG image given PNG data.

	\param data PNG data string.
	\param len PNG data length.

	\return New image, or NULL on failure.
*/
img_t *img_load_png(const char *data, int len)
{
	if(len < 8 || memcmp(data, "\x89PNG\x0D\x0A\x1A\x0A", 8))
	{
		fprintf(stderr, "img_load_png: PNG signature mismatch\n");
		return NULL;
	}

	char *idat_buf = NULL;
	int idat_len = 0;
	const char *dfol = data + 8;
	const char *dend = data + len;

	int x, y, i;

	int img_w = 0, img_h = 0;
	int img_bpc = 0, img_ct = 0;
	int img_cm = 0, img_fm = 0, img_im = 0;

	for(;;)
	{
		// Get chunk region
		if(dend - dfol < 12)
		{
			fprintf(stderr, "img_load_png: file truncated\n");
			if(idat_buf != NULL) free(idat_buf);
			return NULL;
		}

		uint32_t clen = ntohl(*(uint32_t *)dfol);
		dfol += 4;
		const char *cname = dfol;
		dfol += 4;
		
		if((dend - dfol) < clen + 4)
		{
			fprintf(stderr, "img_load_png: file truncated\n");
			if(idat_buf != NULL) free(idat_buf);
			return NULL;
		}

		const char *cend = dfol + clen;
		uint32_t csum = ntohl(*(uint32_t *)cend);
		uint32_t realcsum = crc32(0, (const Bytef *)(dfol - 4), (uInt)(clen + 4));

		if(csum != realcsum)
		{
			fprintf(stderr, "img_load_png: checksum fail\n");
			if(idat_buf != NULL) free(idat_buf);
			return NULL;
		}
		/*
		printf("%c%c%c%c: checksum %08X - calculated %08X\n"
			, cname[0], cname[1], cname[2], cname[3]
			, csum, realcsum);
		*/

		// Parse chunk
		if(!memcmp(cname, "IEND", 4))
		{
			dfol = cend + 4;
			break;

		} else if(!memcmp(cname, "IHDR", 4)) {
			if(clen != 13)
			{
				fprintf(stderr, "img_load_png: invalid IHDR length\n");
				if(idat_buf != NULL) free(idat_buf);
				return NULL;
			}

			img_w = (int)ntohl(*(uint32_t *)(dfol+0));
			img_h = (int)ntohl(*(uint32_t *)(dfol+4));
			img_bpc = (int)(uint8_t)dfol[8];
			img_ct = (int)(uint8_t)dfol[9];
			img_cm = (int)(uint8_t)dfol[10];
			img_fm = (int)(uint8_t)dfol[11];
			img_im = (int)(uint8_t)dfol[12];

			// sanity checks
			if(img_w <= 0 || img_h <= 0 || img_w > 32768 || img_h > 32768
				|| img_w * img_h > (2<<26))
			{
				fprintf(stderr, "img_load_png: image too large or contains a 0 dimension\n");
				if(idat_buf != NULL) free(idat_buf);
				return NULL;
			}

			if(img_bpc != 8)
			{
				fprintf(stderr, "img_load_png: only 8bpc images supported\n");
				if(idat_buf != NULL) free(idat_buf);
				return NULL;
			}

			if(img_ct != 6)
			{
				fprintf(stderr, "img_load_png: only RGBA images supported\n");
				if(idat_buf != NULL) free(idat_buf);
				return NULL;
			}

			if(img_cm != 0)
			{
				fprintf(stderr, "img_load_png: invalid compression method\n");
				if(idat_buf != NULL) free(idat_buf);
				return NULL;
			}

			if(img_im != 0)
			{
				fprintf(stderr, "img_load_png: interlacing not supported\n");
				if(idat_buf != NULL) free(idat_buf);
				return NULL;
			}

			if(img_fm != 0)
			{
				fprintf(stderr, "img_load_png: invalid filter method\n");
				if(idat_buf != NULL) free(idat_buf);
				return NULL;
			}

		} else if(!memcmp(cname, "IDAT", 4)) {
			// Concatenate
			idat_buf = realloc(idat_buf, idat_len + clen);
			memcpy(idat_buf + idat_len, dfol, clen);
			idat_len += clen;

		} else if(!(cname[0] & 0x20)) {
			fprintf(stderr, "img_load_png: unhandled critical chunk %c%c%c%c\n"
				, cname[0], cname[1], cname[2], cname[3]);
			if(idat_buf != NULL) free(idat_buf);
			return NULL;
		}

		dfol = cend + 4;

	}

	if(dend - dfol > 0)
		fprintf(stderr, "img_load_png: warning: excess garbage in PNG file\n");

	// TODO - create actual image
	if(idat_buf == NULL)
	{
		fprintf(stderr, "img_load_png: no IDAT chunks in image\n");
		return NULL;
	} else {
		int unsize = (img_w*4+1)*img_h;
		uint8_t *unpackbuf = malloc(unsize);
		uLongf unlen = (uLongf)unsize;

		if(uncompress((Bytef *)unpackbuf, &unlen, (Bytef *)idat_buf, idat_len)
			|| unlen != (uLongf)unsize)
		{
			fprintf(stderr, "img_load_png: uncompressed size incorrect or unpack failure\n");
			free(unpackbuf);
			free(idat_buf);
			return NULL;
		}

		// filter image
		uint8_t *fx = unpackbuf + 1;
		uint8_t *fa = NULL;
		uint8_t *fb = NULL;
		uint8_t *fc = NULL;

		for(y = 0; y < img_h; y++)
		{
			uint8_t *nfb = fx;
			uint8_t typ = fx[-1];
			fa = fc = NULL;

			// we're not loading PNG files all the time
			// so it's a good time to rank purity over speed
			for(x = 0; x < img_w; x++)
			{
				for(i = 0; i < 4; i++)
				{
					switch(typ)
					{
						case 0: // None
							break;
						case 1: // Sub
							fx[i] += (fa == NULL ? 0 : fa[i]);
							break;
						case 2: // Up
							fx[i] += (fb == NULL ? 0 : fb[i]);
							break;
						case 3: // Average
							fx[i] += (((int)(fa == NULL ? 0 : fa[i]))
								+ (int)(fb == NULL ? 0 : fb[i]))
								/ 2;
							break;
						case 4: // Paeth
						{
							int pia = (int)(fa == NULL ? 0 : fa[i]);
							int pib = (int)(fb == NULL ? 0 : fb[i]);
							int pic = (int)(fc == NULL ? 0 : fc[i]);

							int p = pia + pib - pic;
							int pa = (p > pia ? p - pia : pia - p);
							int pb = (p > pib ? p - pib : pib - p);
							int pc = (p > pic ? p - pic : pic - p);

							int pr = 0;
							if(pa <= pb && pa <= pc) pr = pia;
							else if(pb <= pc) pr = pib;
							else pr = pic;

							fx[i] += pr;
						} break;
						default:
							fprintf(stderr, "img_load_png: invalid filter\n");
							free(unpackbuf);
							free(idat_buf);
							return NULL;
					}
				}

				fa = fx; fx += 4;
				if(fb != NULL) { fc = fb; fb += 4; }
			}

			fb = nfb;
			fx++; // Skip filter mode byte
		}

		// Create image
		img_t *img = img_new(img_w, img_h);
		for(y = 0; y < img_h; y++)
		{
			uint8_t *p = (unpackbuf + y*(4*img_w+1) + 1);
			uint8_t *d = (uint8_t *)(img->data + img_w*y);
			for(x = 0; x < img_w; x++)
			{
				d[0] = p[2];
				d[1] = p[1];
				d[2] = p[0];
				d[3] = p[3];
				p += 4;
				d += 4;
			}
		}

		free(unpackbuf);
		free(idat_buf);
		return img;
	}
}

