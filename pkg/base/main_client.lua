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

-- TODO: wrap loadfile

-- load images
img_tiles = common.fetch("png", "pkg/base/gfx/hello.png")
img_font = common.fetch("png", "pkg/base/gfx/font-mini.png")

-- preload libraries
local k,v
local libload = {}
for k,v in pairs({
	"pkg/base/constants.lua",
	"pkg/base/lib/gui.lua",
	"pkg/base/lib/timer.lua",
	"pkg/base/obj/door.lua",
}) do libload[k] = loadfile(v) end

-- run libraries
for k,v in pairs(libload) do v() end

testlua = loadfile("pkg/base/test.lua") -- regression testing

local test_map = {
	"     ####           ",
	"     #..#           ",
	"     #..#           ",
	"  ####..#           ",
	"  =.=...#           ",
	"  #####=#           ",
	"     #..####        ",
	"     #.$.=.=        ",
	"     #..####        ",
	"   ###=#########    ",
	"   #...........=    ",
	"   #...........=    ",
	"   #############    ",
}

test_map_trn = {
	[" "] = TURF.WATER,
	["."] = TURF.FLOOR,
	["$"] = TURF.FLOOR,
	["#"] = TURF.WALL,
	["="] = TURF.WALL,
}

map_walls = {}
map_doors = {}
map_vis = {}
local i,j
for i=1,128 do
	map_walls[i] = {}
	map_doors[i] = {}
	map_vis[i] = {}
	for j=1,128 do
		map_vis[i][j] = TURF.WATER
	end
end

door_list = {}
wall_list = {}

-- Lua is 1-based. Ugh. But we'll cope.
print(#(test_map[1]), #test_map)
--map = common.map_new(#(test_map[1]), #test_map)
map = common.map_new(128, 128)

local x,y
for y=1,#test_map do
	local s = test_map[y]
	for x=1,#s do
		local c = s:sub(x,x)
		local ctype = test_map_trn[c]
		--print(x, y, c, ctype)
		map_vis[y][x] = ctype
		common.turf_set_type(map, x, y, ctype)
		common.turf_reset_gas(map, x, y)

		if ctype == TURF.WALL then
			wall_list[#wall_list+1] = {x, y}
			map_walls[y][x] = true
			if c == "=" then
				local door = door_new {x = x, y = y}
				door_list[#door_list+1] = door
				map_doors[y][x] = door
			end
		end

		if c == "$" then
			drain_x = x
			drain_y = y
		end
	end
end


poop = 0.2
deadwall = nil

local sec_beg = nil

function set_sec_beg(sec)
	if not sec_beg then
		sec_beg = sec
		math.randomseed(sec)
	end
end

function hook_render(sec_current, sec_delta)
	local x,y
	local mx, my, _, mb

	mx, my, _, _, mb = common.mouse_get()

	set_sec_beg(sec_current)

	for y=1,math.min(1+13-1,#test_map) do
	for x=1,math.min(1+20-1,#(test_map[1])) do
		local tx, ty
		local typ = map_vis[y][x]
		local water = common.turf_get_gas(map, x, y, GAS.WATER)
		
		local door = map_doors[y][x]
		if typ == TURF.WATER then
			tx, ty = 0, 0
		elseif door or typ == TURF.FLOOR then
			tx, ty = 1, 0
		elseif typ == TURF.WALL then
			tx, ty = 0, 2
			water = 0
			local t0 = (x >= #(test_map[1]) and TURF.WATER) or map_vis[y][x+1]
			local t1 = (y >= #test_map and TURF.WATER) or map_vis[y+1][x]
			local t2 = (x <= 1 and TURF.WATER) or map_vis[y][x-1]
			local t3 = (y <= 1 and TURF.WATER) or map_vis[y-1][x]
			
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

		if door then
			door.draw(sec_current, sec_delta, (x-1)*16, (y-1)*16)
		end
	end
	end

	local cx, cy
	cx = math.floor(mx/16)+1
	cy = math.floor(my/16)+1

	common.draw_rect_outl(nil, (cx-1)*16, (cy-1)*16, (cx-1)*16+15, (cy-1)*16+15, 0xFF880000)

	local xbase = ((sec_current - sec_beg) % 5.0) / 5.0
	xbase = xbase * (320 + 2*100)
	xbase = xbase - 100
	if deadwall then
		local door = door_list[deadwall]
		local x = door.x - 100/32
		local y = door.y - 100/32
		common.img_blit(nil, 320-100, 0, BF_AM_THRES,
			(x-1)*16, (y-1)*16, 100, 100, nil)
	end

	local s = "All systems nominal."
	common.draw_rect_fill(nil, 0, 0, 4 + 6*#s, 12, 0xFF225522)
	common.draw_rect_outl(nil, 0, 0, 4 + 6*#s, 12, 0xFF88AA88)
	puts(2, 2, s)
end

tmr_atmos = nil
function hook_tick(sec_current, sec_delta)
	set_sec_beg(sec_current)

	tmr_atmos = tmr_atmos or timer_new(function(sec)
		common.map_tick_atmos(map)
		common.turf_set_gas(map, drain_x, drain_y, GAS.WATER,
			common.turf_get_gas(map, drain_x, drain_y, GAS.WATER) * 0.8)
	end, sec_current, 1.0/50.0)

	local i
	for i=1,#door_list do
		local door = door_list[i]
		door.tick(sec_current, sec_delta)
	end

	tmr_atmos(sec_current)
	poop = poop - sec_delta
	while poop <= 0 do
		poop = poop + 3.0
		local x, y
		local door

		if deadwall then
			door = door_list[deadwall]
			door.close(sec_current)
		end

		deadwall = math.floor(math.random() * #door_list) + 1
		door = door_list[deadwall]
		x, y = door.x, door.y
		door.open(sec_current)
	end

end

function hook_mouse(x, y, button, state)
	print("mouse", x, y, button, state)
	print("more crap", common.mouse_get())
end

print(testlua("this", 3.14, "meow"))
print(testlua("arsethis", 3.14, "meow"))


