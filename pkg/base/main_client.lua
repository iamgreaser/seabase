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

testlua = loadfile("pkg/base/test.lua")
img_tiles = common.fetch("png", "pkg/base/gfx/hello.png")
img_font = common.block(common.fetch("png", "pkg/base/gfx/font-mini.png"))

function timer_new(fn, sec_base, interval)
	return function(sec)
		while sec > sec_base do
			fn(sec_base)
			sec_base = sec_base + interval
		end
	end
end

function door_new(cfg)
	local this = {
		x = cfg.x, y = cfg.y,

		tmr_openclose = nil,
		openness = 0,
		open_state = false,
		opening = false,
		closing = false,
	}

	--map_vis[this.y][this.x] = TURF.FLOOR

	this.this = this

	function this.f_open()
		this.openness = this.openness + 1
		if this.openness > 7 then
			this.openness = 7
			this.open_state = true
			this.opening = false
			this.tmr_openclose = nil
			common.turf_set_type(map, this.x, this.y, TURF.FLOOR)
		end
	end

	function this.f_close()
		this.openness = this.openness - 1
		if this.openness < 0 then
			this.openness = 0
			this.open_state = false
			this.closing = false
			this.tmr_openclose = nil
			common.turf_set_type(map, this.x, this.y, TURF.WALL)
		end
	end

	function this.tick(sec_current, sec_delta)
		if this.tmr_openclose then
			this.tmr_openclose(sec_current, sec_delta)
		end
	end

	function this.open(sec_current)
		if (not this.open_state) or (this.closing) then
			this.closing = false
			this.opening = true
			this.tmr_openclose = timer_new(this.f_open, sec_current, 1.0/8.0)
		end
	end

	function this.close(sec_current)
		if (this.open_state) or (this.opening) then
			this.opening = false
			this.closing = true
			this.tmr_openclose = timer_new(this.f_close, sec_current, 1.0/8.0)
		end
	end

	function this.draw(sec_current, sec_delta, bx, by)
		common.img_blit(img_tiles, bx, by, BF_AM_THRES,
			16*this.openness, 16*3, 16, 16)
	end

	return this
end

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

print(testlua("this", 3.14, "meow"))
print(testlua("arsethis", 3.14, "meow"))


