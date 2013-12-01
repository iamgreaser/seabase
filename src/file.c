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
			//fcount--; // I don't know why I have to do this, I just do.
			// Turns out doing that makes it break.
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
	\brief Check the security

	\param fname Name of file to be read.
	\param len Pointer to return length of data.
	\return Newly-allocated pointer to data, or NULL on error.
*/
int file_sec_check(const char *fname, int is_client, int is_write)
{
	const char *v;

	// Check some things
	if(fname == NULL)
		return SEC_FORBID;
	if(strstr("fname", "..") != NULL)
		return SEC_FORBID;
	
	// Check if all chars are valid
	for(v = fname; *v != '\0'; v++)
	{
		if(*v == '-') continue;
		if(*v == '.') continue;
		if(*v == '/') continue;
		if(*v >= '0' && *v <= '9') continue;
		if(*v >= 'a' && *v <= 'z') continue;
		if(*v == '_') continue;
		if(*v >= 'A' && *v <= 'Z') continue;

		printf("bad char %c\n", *v);
		return (is_client ? SEC_REMOTE : SEC_FORBID);
	}
	
	// Specific paths
	if(strlen(fname) >= 4 && (!is_write) && !memcmp(fname, "pkg/", 4))
		return (is_client ? SEC_REMOTE : SEC_LOCAL);
	if(strlen(fname) >= 11 && is_client && (!is_write) && !memcmp(fname, "clsave/pub/", 11))
		return SEC_LOCAL;
	if(strlen(fname) >= 11 && is_client && !memcmp(fname, "clsave/vol/", 11))
		return SEC_LOCAL;
	if(strlen(fname) >= 11 && (!is_client) && (!is_write) && !memcmp(fname, "svsave/pub/", 11))
		return SEC_LOCAL;
	if(strlen(fname) >= 11 && (!is_client) && !memcmp(fname, "svsave/vol/", 11))
		return SEC_LOCAL;
	
	// We aren't in the specific paths anymore!
	return (is_client ? SEC_REMOTE : SEC_FORBID);
}

/**
	\brief Lua helper: Parse file data

	\param L Lua state.
	\param data Pointer to data.
	\param len Length of data.
	\param fmt Format of data.

	\return Pushes the required object onto the Lua stack, if it hasn't thrown an error.
*/
void file_parse_any(lua_State *L, const char *data, int len, const char *fmt, const char *fname)
{
	// TODO!
	if(!strcmp(fmt, "lua"))
	{
		lua_getglobal(L, "loadstring");
		lua_pushlstring(L, data, len);
		lua_pushstring(L, fname);
		lua_call(L, 2, 2);
		if(!lua_isfunction(L, -2))
			luaL_error(L, lua_tostring(L, -1));
		lua_remove(L, -1);
	} else if(!strcmp(fmt, "png")) {
		img_t *img = img_load_png(data, len);
		if(img == NULL)
			luaL_error(L, "image failed to parse");
		img_provide_ud(L, img);
	} else {
		luaL_error(L, "invalid format \"%s\"", fmt);
	}
}

/**
	\brief Load a file using the appropriate method.

	FIXME: May need an API change to drop objects directly onto the Lua stack.

	\param fname Name of file to be read.
	\param len Pointer to return length of data.
	\return Newly-allocated pointer to data, or NULL on error.
*/
char *file_get(const char *fname, int *len)
{
	// TODO: filename security and whatnot
	switch(file_sec_check(fname, is_client, 0))
	{
		case SEC_LOCAL:
			printf("accepted %s\n", fname);
			return file_get_direct(fname, len);
		case SEC_REMOTE:
			switch(file_sec_check(fname, 0, 0))
			{
				case SEC_LOCAL:
					printf("remote %s\n", fname);
					return file_get_direct(fname, len);
				case SEC_REMOTE:
					printf("doubly-remote?! FORBID %s\n", fname);
					return NULL;
				case SEC_FORBID:
					printf("forbidden %s\n", fname);
					return NULL;
				default:
					eprintf("EDOOFUS: file_sec_check returned invalid enum!\n");
					fflush(stderr);
					abort();
					return NULL;
			} break;
		case SEC_FORBID:
			printf("forbidden %s\n", fname);
			return NULL;
		default:
			eprintf("EDOOFUS: file_sec_check returned invalid enum!\n");
			fflush(stderr);
			abort();
			return NULL;
	}
}

