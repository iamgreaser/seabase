#include "common.h"

/**
	\brief Load a file directly from the filesystem, bypassing filename security.

	\param fname Name of file to be read.
	\param len Pointer to return length of data.
	\return Newly-allocated pointer to data, or NULL on error.
*/
char *file_get_direct(const char *fname, int *len)
{
	FILE *fp = fopen(fname, "rb");

	if(fp == NULL)
	{
		perror("file_get_direct.fopen");
		return NULL;
	}

	int buflen = 1024;
	int bufoffs = 0;
	char *buf = malloc(buflen);

	for(;;)
	{
		int fcount = fread(buf+bufoffs, 1, buflen-bufoffs, fp);

		if(fcount == -1)
		{
			perror("file_get_direct.fread");
			free(buf);
			fclose(fp);
			return NULL;
		}

		if(fcount == buflen-bufoffs)
		{
			bufoffs = buflen;
			buflen = (buflen*3/2+1);

			if(buflen < bufoffs+1)
				buflen = bufoffs+1;

			buf = realloc(buf, buflen);
		} else {
			fcount--; // I don't know why I have to do this, I just do.
			//printf("%i %i\n", bufoffs, fcount);
			*len = bufoffs + fcount;
			buf[*len] = '\x00';
			break;
		}
	}

	fclose(fp);
	//printf("file: [%s]\n", buf);

	return buf;
}

/**
	\brief Load a file using the appropriate method.

	\param fname Name of file to be read.
	\param len Pointer to return length of data.
	\return Newly-allocated pointer to data, or NULL on error.
*/
char *file_get(const char *fname, int *len)
{
	// TODO: filename security and whatnot
	return file_get_direct(fname, len);
}

/**
	\brief Compile a file using file_get.

	\param S Relevant Squirrel virtual machine.
	\param fname Name of file to be read.
	\return Boolean indicating if the operation succeeded.
*/
int hsq_compile(HSQUIRRELVM S, const char *fname)
{
	int blen = 0;
	char *buf = file_get(fname, &blen);

	if(buf == NULL)
		return 0;

	SQRESULT ret = sq_compilebuffer(S, buf, blen, fname, SQTrue);
	free(buf);

	return SQ_SUCCEEDED(ret);
}

