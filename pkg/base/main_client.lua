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
	"pkg/base/obj/floor.lua",
	"pkg/base/obj/wall.lua",
}) do libload[#libload+1] = loadfile(v) end
for k,v in pairs({
	"door", "floor", "wall",
}) do libload[#libload+1] = loadfile("pkg/base/obj/" .. v .. ".lua") end

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
	[" "] = function(x, y)
		return TURF.WATER, {}
	end,
	["."] = function (x, y)
		return TURF.FLOOR, {
			floor = {floor_new {x=x, y=y}},
		}
	end,
	["$"] = function (x, y)
		return TURF.FLOOR, {
			floor = {floor_new {x=x, y=y},},
			-- TODO: add drain
		}
	end,
	["#"] = function (x, y)
		return TURF.WALL, {
			floor = {floor_new {x=x, y=y},},
			wall = {wall_new {x=x, y=y},},
		}
	end,
	["="] = function (x, y)
		return TURF.WALL, {
			floor = {floor_new {x=x, y=y},},
			wall = {door_new {x=x, y=y},},
		}
	end,
}

obj_list = {}
map_tiles = {}
map_vis = {}
local i,j
for i=1,128 do
	map_tiles[i] = {}
	map_vis[i] = {}
	for j=1,128 do
		map_tiles[i][j] = {floor = {}, obj = {}, wall = {}}
		map_vis[i][j] = TURF.WATER
	end
end

tick_list = {}

-- Lua is 1-based. Ugh. But we'll cope.
print(#(test_map[1]), #test_map)
--map = common.map_new(#(test_map[1]), #test_map)
map = common.map_new(128, 128)

local x,y
for y=1,#test_map do
	local s = test_map[y]
	for x=1,#s do
		local c = s:sub(x,x)
		local ctype, cobjs = test_map_trn[c](x, y)

		cobjs.floor = cobjs.floor or {}
		cobjs.obj = cobjs.obj or {}
		cobjs.wall = cobjs.wall or {}

		map_tiles[y][x] = cobjs

		local _,l,o
		for _,l in pairs({"floor", "obj", "wall"}) do
			for _,o in pairs(cobjs[l]) do
				if o.tick then
					tick_list[#tick_list+1] = o
				end
			end
		end

		map_vis[y][x] = ctype
		common.turf_set_type(map, x, y, ctype)
		common.turf_reset_gas(map, x, y)
	end
end

local sec_beg = nil

function set_sec_beg(sec)
	if not sec_beg then
		sec_beg = sec
		math.randomseed(sec)
	end
end

function obj_get_top(cobjs)
	if #cobjs.wall > 0 then return cobjs.wall[#cobjs.wall] end
	if #cobjs.obj > 0 then return cobjs.obj[#cobjs.obj] end
	if #cobjs.floor > 0 then return cobjs.floor[#cobjs.floor] end
	return nil
end

wpopups = {}

function popup_clear()
	wpopups = {}
end

function popup_do(x, y)
	local items = {}

	local function addi(text, fn)
		items[#items+1] = wchild(widget.box {
			children = { wchild(widget.text {u_text = text}) },
			ev_mouse_button = function(bx, by, ax, ay, button, state)
				if button == 0 and not state then
					return fn()
				end
			end})
	end

	local cx, cy = math.floor(x/16)+1, math.floor(y/16)+1
	local cobjs = map_tiles[cy][cx] 
	local typ = common.turf_get_type(map, cx, cy)
	local obj = obj_get_top(cobjs)

	if obj then
		addi("Examine", obj.examine)
		if obj.actions then
			local _,t
			for _,t in pairs(obj.actions) do
				addi(t.text, t.fn)
			end
		end
	else
		addi("Examine", function ()
			cons_print("EX: That's the ocean.")
			cons_print("Keep out of reach of batteries.")
		end)
	end

	local this = widget.box {
		maxw = 100,
		layout = widget.layout.vbox_flow {},
		children = items,
	}

	this.u_x = x
	this.u_y = y
	this.pack()
	local sw, sh = common.img_get_dims(nil)
	if this.h + this.u_y > sh then this.u_y = sh - this.h end
	if this.w + this.u_x > sw then this.u_x = sw - this.w end
	wpopups[#wpopups + 1] = this
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
		
		local cobjs = map_tiles[y][x]
		water = math.max(0, math.min(1, water))
		water = math.floor(water * 16 - 0.5)

		local _,l,o
		for _,l in pairs({"floor", "obj", "wall"}) do
			for _,o in pairs(cobjs[l]) do
				o.draw(sec_current, sec_delta, (x-1)*16, (y-1)*16, map_vis, test_map)
			end

			if l == "floor" and water >= 0 then
				common.img_blit(img_tiles, (x-1)*16, (y-1)*16, BF_AM_THRES,
					16*water, 16*1, 16, 16)
			end
		end
	end
	end

	local cx, cy
	cx = math.floor(mx/16)+1
	cy = math.floor(my/16)+1

	common.draw_rect_outl(nil, (cx-1)*16, (cy-1)*16, (cx-1)*16+15, (cy-1)*16+15, 0xFF880000)

	local _,wi
	for _,wi in pairs(wpopups) do
		wi.draw(wi.u_x, wi.u_y)
	end

	local xbase = ((sec_current - sec_beg) % 5.0) / 5.0
	xbase = xbase * (320 + 2*100)
	xbase = xbase - 100
end

tmr_atmos = nil
function hook_tick(sec_current, sec_delta)
	set_sec_beg(sec_current)

	tmr_atmos = tmr_atmos or timer_new(function(sec)
		common.map_tick_atmos(map)
	end, sec_current, 1.0/50.0)

	local i
	for i=1,#tick_list do
		local obj = tick_list[i]
		obj.tick(sec_current, sec_delta)
	end

	tmr_atmos(sec_current)
end

function hook_mouse(x, y, button, state)
	if button == 2 then
		if state then
			popup_clear()
			popup_do(x, y)
		end
	else
		if not state then
			local _,wi
			for _,wi in pairs(wpopups) do
				widget_mouse_button(wi, wi.u_x, wi.u_y, x, y, button, state)
			end

			popup_clear()
		end
	end
end

print(testlua("this", 3.14, "meow"))
print(testlua("arsethis", 3.14, "meow"))


