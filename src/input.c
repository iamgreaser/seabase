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

int mouse_x = -1;
int mouse_y = -1;
int mouse_lx = -1;
int mouse_ly = -1;
int mouse_b = 0;
int mouse_needs_reset = 1;

/**
	\brief Pushes the current mouse state onto the Lua stack.

	\param L Lua state.

	\returns 5 parameters on the Lua stack (x, y, dx, dy, b).
*/
void input_mouse_get_lua(lua_State *L)
{
	// Push x, y
	lua_pushinteger(L, mouse_x);
	lua_pushinteger(L, mouse_y);

	// Push dx, dy
	if(mouse_needs_reset)
	{
		// Drop dx, dy if mouse needs a reset
		lua_pushinteger(L, 0);
		lua_pushinteger(L, 0);
		mouse_needs_reset = 0;
	} else {
		lua_pushinteger(L, mouse_x - mouse_lx);
		lua_pushinteger(L, mouse_y - mouse_ly);
	}

	lua_pushinteger(L, mouse_b);

	// Update lx, ly
	mouse_lx = mouse_x;
	mouse_ly = mouse_y;
}

/**
	\brief Informs the input subsystem of a mouse position change.

	\param L Lua state.
	\param x Mouse x coordinate.
	\param y Mouse y coordinate.
*/
void input_mouse_update_pos(lua_State *L, int x, int y)
{
	(void)L;
	mouse_x = x/3;
	mouse_y = y/3;
}

/**
	\brief Informs the input subsystem of a mouse button state change.

	\param L Lua state.
	\param x Mouse x coordinate.
	\param y Mouse y coordinate.
	\param b Button index.
	\param state Boolean state of button.
*/
void input_mouse_update_button(lua_State *L, int x, int y, int b, int state)
{
	// Update state
	if(state) mouse_b |=  (1<<b);
	else      mouse_b &= ~(1<<b);
	mouse_x = x/3;
	mouse_y = y/3;

	// Tell Lua
	lua_getglobal(L, "hook_mouse");
	if(lua_isnil(L, -1))
	{
		lua_remove(L, -1);
	} else {
		lua_pushinteger(L, mouse_x);
		lua_pushinteger(L, mouse_y);
		lua_pushinteger(L, b);
		lua_pushboolean(L, state);
		// TODO: pcall
		lua_call(L, 4, 0);
	}
}


