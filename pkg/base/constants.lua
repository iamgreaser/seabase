--[[
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
]]

BF_M_AMODE  = 0x00000003
BF_AM_DIRECT   = 0x00000000
BF_AM_THRES    = 0x00000001
BF_AM_BLEND    = 0x00000002
BF_AM_DITHER   = 0x00000003

TURF = {
	WATER = 0,
	FLOOR = 1,
	WALL = 2,
}

GAS = {
	WATER = 1,

	O2 = 2,
	N2 = 3,
	CO2 = 4,
	CH4 = 5,
}

DIR = {
	E = 0x01,
	S = 0x02,
	W = 0x04,
	N = 0x08,

	EAST = 0x01,
	SOUTH = 0x02,
	WEST = 0x04,
	NORTH = 0x08,
}

LAYER = {
	FLOOR = 1,
	OBJ = 2,
	WALL = 3,
}

