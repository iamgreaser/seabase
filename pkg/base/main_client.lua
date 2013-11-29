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

--[[
Something to get your knickers in a knot about.

Type this into your shell.
local b = "hi"; function a() { print(b) } a();

This works fine in the Squirrel shell.
However, this breaks horribly when you do it here.

If I can't get that working soon, I'm going to dump Squirrel and move back to Lua,
because this is just complete horseshit.

--GM

OK, porting to Lua because fuck Squirrel.

--GM

]]
BF_M_AMODE  = 0x00000003
BF_AM_DIRECT   = 0x00000000
BF_AM_THRES    = 0x00000001
BF_AM_BLEND    = 0x00000002
BF_AM_DITHER   = 0x00000003

img_tiles = common.fetch("png", "pkg/base/gfx/hello.png")
img_font = common.fetch("png", "pkg/base/gfx/font-mini.png")

testlua = loadfile("pkg/base/test.lua")

function puts(x, y, s)
	local i
	for i=1,#s do
		local c = s:byte(i) - 0x20
		common.img_blit(img_font, x, y, BF_AM_THRES, c*6, 0, 6, 8)
		x = x + 6
	end
end

local test_map = {
	"     ####           ",
	"     #..#           ",
	"     #..#           ",
	"  ####..#           ",
	"  #.....#           ",
	"  #######           ",
	"     #..####        ",
	"     #.$...#        ",
	"     #..####        ",
	"   ########         ",
	"   #......#         ",
	"   #......#         ",
	"   ########         ",
}

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

test_map_trn = {
	[" "] = TURF.WATER,
	["."] = TURF.FLOOR,
	["$"] = TURF.FLOOR,
	["#"] = TURF.WALL,
}

wall_list = {}

-- Lua is 1-based. Ugh. But we'll cope.
print(#(test_map[1]), #test_map)
map = common.map_new(#(test_map[1]), #test_map)

local x,y
for y=1,#test_map do
	local s = test_map[y]
	for x=1,#s do
		local c = s:sub(x,x)
		local ctype = test_map_trn[c]
		--print(x, y, c, ctype)
		common.turf_set_type(map, x, y, ctype)
		common.turf_reset_gas(map, x, y)

		if ctype == TURF.WALL then
			wall_list[#wall_list+1] = {x, y}
		end

		if c == "$" then
			drain_x = x
			drain_y = y
		end
	end
end


poop = 1
deadwall = math.floor(math.random() * #wall_list) + 1
local x,y
x = wall_list[deadwall][1]
y = wall_list[deadwall][2]
common.turf_set_type(map, x, y, TURF.FLOOR)

local sec_beg = nil

function set_sec_beg(sec)
	if not sec_beg then
		sec_beg = sec
		math.randomseed(sec)
	end
end

function hook_render(sec_current, sec_delta)
	local x,y

	set_sec_beg(sec_current)

	for y=1,#test_map do
	for x=1,#(test_map[1]) do
		local tx, ty
		local typ = common.turf_get_type(map, x, y)
		local water = common.turf_get_gas(map, x, y, GAS.WATER)
		
		if typ == TURF.WATER then
			tx, ty = 0, 0
		elseif typ == TURF.FLOOR then
			tx, ty = 1, 0
		elseif typ == TURF.WALL then
			tx, ty = 0, 2
			water = 0
			local t0 = (x >= #(test_map[1]) and TURF.WATER) or common.turf_get_type(map, x+1, y)
			local t1 = (y >= #test_map and TURF.WATER) or common.turf_get_type(map, x, y+1)
			local t2 = (x <= 1 and TURF.WATER) or common.turf_get_type(map, x-1, y)
			local t3 = (y <= 1 and TURF.WATER) or common.turf_get_type(map, x, y-1)
			
			if t0 == TURF.WALL then tx = tx + 1 end
			if t1 == TURF.WALL then tx = tx + 2 end
			if t2 == TURF.WALL then tx = tx + 4 end
			if t3 == TURF.WALL then tx = tx + 8 end
		end
		water = math.max(0, math.min(1, water))
		water = math.floor(water * 16 - 0.5)
		if x ~= drain_x or y ~= drain_y then
			common.img_blit(img_tiles, (x-1)*16, (y-1)*16, BF_AM_THRES,
				16*tx, 16*ty, 16, 16)
		end
		if water >= 0 then
			common.img_blit(img_tiles, (x-1)*16, (y-1)*16, BF_AM_THRES,
				16*water, 16*1, 16, 16)
		end
	end
	end

	local xbase = ((sec_current - sec_beg) % 5.0) / 5.0
	xbase = xbase * (320 + 2*100)
	xbase = xbase - 100
	local x = wall_list[deadwall][1] - 100/32
	local y = wall_list[deadwall][2] - 100/32
	common.img_blit(nil, 320-100, 0, BF_AM_THRES,
		(x-1)*16, (y-1)*16, 100, 100, nil)
	

	local s = "All systems nominal."
	common.draw_rect_fill(nil, 0, 0, 4 + 6*#s, 12, 0xFF225522)
	common.draw_rect_outl(nil, 0, 0, 4 + 6*#s, 12, 0xFF88AA88)
	puts(2, 2, s)
end

function hook_tick(sec_current, sec_delta)
	set_sec_beg(sec_current)

	poop = poop - 1
	if poop <= 0 then
		poop = 100
		local x, y
		x = wall_list[deadwall][1]
		y = wall_list[deadwall][2]
		common.turf_set_type(map, x, y, TURF.WALL)
		common.turf_reset_gas(map, x, y)

		deadwall = math.floor(math.random() * #wall_list) + 1
		x = wall_list[deadwall][1]
		y = wall_list[deadwall][2]
		common.turf_set_type(map, x, y, TURF.FLOOR)
	end

	common.turf_set_gas(map, drain_x, drain_y, GAS.WATER,
		common.turf_get_gas(map, drain_x, drain_y, GAS.WATER) * 0.8)
end

print(testlua("this", 3.14, "meow"))
print(testlua("arsethis", 3.14, "meow"))


