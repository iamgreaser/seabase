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
	WATER = 0,

	O2 = 1,
	N2 = 2,
	CO2 = 3,
	CH4 = 4,
}
