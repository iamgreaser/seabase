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

		printf("%c%c%c%c: checksum %08X - calculated %08X\n"
			, cname[0], cname[1], cname[2], cname[3]
			, csum, realcsum);

		dfol = cend + 4;

		if(!memcmp(cname, "IEND", 4))
			break;
	}

	if(dend - dfol > 0)
		fprintf(stderr, "img_load_png: warning: excess garbage in PNG file\n");

	// TODO - create actual image
	return NULL;
}

