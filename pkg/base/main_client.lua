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

-- adjust the GC - it's kinda necessary
collectgarbage("setpause", 105) -- wait for a 5% increase - 100% inc is too high

cam_cx, cam_cy = 0, 0

-- load map
map_loader_data = {
	base = common.fetch("png", "pkg/maps/firstpost/base.png"),
	atmosia = common.fetch("png", "pkg/maps/firstpost/atmosia.png"),
}

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
	"pkg/base/obj/pipe.lua",
	"pkg/base/obj/table.lua",
	"pkg/base/obj/wall.lua",
}) do libload[#libload+1] = loadfile(v) end
for k,v in pairs({
	"door", "floor", "wall",
}) do libload[#libload+1] = loadfile("pkg/base/obj/" .. v .. ".lua") end

-- run libraries
for k,v in pairs(libload) do v() end

testlua = loadfile("pkg/base/test.lua") -- regression testing

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

map_img_trn = {}
map_img_trn.base = {
	[0x00000000] = function(x, y)
		return TURF.WATER, {}
	end,
	[0xFF999999] = function (x, y)
		return TURF.FLOOR, {
			floor = {floor_new {x=x, y=y}},
		}
	end,
	[0xFF333333] = function (x, y)
		return TURF.FLOOR, {
			floor = {floor_new {x=x, y=y, has_tiles=false}},
		}
	end,
	[0xFF666666] = function (x, y)
		return TURF.WALL, {
			floor = {floor_new {x=x, y=y},},
			wall = {wall_new {x=x, y=y},},
		}
	end,
	[0xFF00AA00] = function (x, y)
		return TURF.WALL, {
			floor = {floor_new {x=x, y=y},},
			wall = {door_new {x=x, y=y},},
		}
	end,
	[0xFFFF5500] = function (x, y)
		return TURF.WALL, {
			-- TODO: use airlock door texture
			floor = {floor_new {x=x, y=y},},
			wall = {door_new {x=x, y=y},},
		}
	end,
	[0xFFAA0000] = function (x, y)
		return TURF.FLOOR, {
			-- TODO: use waterlock door texture
			floor = {floor_new {x=x, y=y},},
			wall = {door_new {x=x, y=y, open=true, u_speed=0.5, water_lock=true},},
		}
	end,
	[0xFFFFFFFF] = function (x, y)
		return TURF.FLOOR, {
			floor = {floor_new {x=x, y=y},},
			obj = {table_new {x=x, y=y},},
		}
	end,
	[0xFFFF00FF] = function (x, y)
		cam_x = x*16
		cam_y = y*16
		return TURF.FLOOR, {
			floor = {
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
				floor_new {x=x, y=y},
			},
		}
	end,
}

tick_list = {}

-- Lua is 1-based. Ugh. But we'll cope.
map = common.map_new(128, 128)

local x,y
for y=1,#map_vis do
	for x=1,#map_vis[y] do
		local c = common.img_get_pixel(map_loader_data.base, x, y)
		local c_atmosia = common.img_get_pixel(map_loader_data.atmosia, x, y)
		if c <= 0x80000000 then c = 0x00000000 end
		--print(string.format("%08X", c), map_img_trn[c])
		local ctype, cobjs = map_img_trn.base[c](x, y)

		if c_atmosia <= 0x01000000 then c_atmosia = 0
		else c_atmosia = c_atmosia % 0x1000000 end

		cobjs.floor = cobjs.floor or {}
		cobjs.obj = cobjs.obj or {}
		cobjs.wall = cobjs.wall or {}

		if c_atmosia == 0x00FFFF then
			table.insert(cobjs.floor, pipe_new {x=x, y=y, subtype=PIPE.PIPE})
		elseif c_atmosia == 0xFFFF00 then
			table.insert(cobjs.floor, pipe_new {x=x, y=y, subtype=PIPE.VENT})
		elseif c_atmosia == 0x0000FF then
			table.insert(cobjs.floor, pipe_new {x=x, y=y, subtype=PIPE.T_WATER_IN})
		elseif c_atmosia == 0xFF0000 then
			table.insert(cobjs.floor, pipe_new {x=x, y=y, subtype=PIPE.T_AIR_OUT})
		end

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

function obj_has_any(cobjs, name)
	local _,l,o
	for _,l in pairs({"floor", "obj", "wall"}) do
		for _,o in pairs(cobjs[l]) do
			if o[name] then return true end
		end
	end
	return false
end
function obj_get_top(cobjs)
	if #cobjs.wall > 0 then return cobjs.wall[#cobjs.wall] end
	if #cobjs.obj > 0 then return cobjs.obj[#cobjs.obj] end
	if #cobjs.floor > 0 then return cobjs.floor[#cobjs.floor] end
	return {fail = true}
end

do
	local clist = {}
	
	local function addi(tx, ty, num, is_sel)
		local function addi_drawobj()
			local this = widget.drawobj {u_obj = true}

			function this.on_draw(img, bx, by, bw, bh, ax, ay, ...)
				common.img_blit(img_tiles, bx, by, BF_AM_THRES,
					16*tx, 16*ty, 16, 16, img)
			end

			return { wchild(this), wchild(widget.text {u_text = "" .. num})}
		end

		clist[#clist+1] = wchild(widget.box {
			cbg = is_sel and 0xFF800000,
			layout = widget.layout.vbox_flow {},
			children = addi_drawobj(),
		})
	end
	
	wtiles = widget.layout.hbox_flow {}
	addi(13, 1, 1)
	addi(1, 0, 2)
	addi(2, 0, 3)
	addi(0, 2, 4)
	addi(0, 3, 5, true)
	addi(7, 4, 6)
	addi(0, 6, 7)
	addi(15, 7, 8)
	addi(5, 0, 9)
	addi(3, 0, "-")
	addi(4, 0, "=")
	wtiles.add_children_batch({children = clist})
	wtiles.pack()
end

wpopups = {}

function popup_clear()
	wpopups = {}
end

function popup_do(x, y)
	local sw, sh = common.img_get_dims(nil)
	local csx, csy = cam_x - math.floor(sw/2), cam_y - math.floor(sh/2)
	csx = math.max(0, math.min(16*#map_vis[1] - sw, csx))
	csy = math.max(0, math.min(16*#map_vis - sh, csy))

	local cx, cy = math.floor((x+csx)/16)+1, math.floor((y+csy)/16)+1
	local cobjs = map_tiles[cy][cx] 
	local typ = common.turf_get_type(map, cx, cy)

	local function do_pop(this, x, y)
		this.u_x = x
		this.u_y = y
		this.pack()
		if this.h + this.u_y > sh then this.u_y = sh - this.h end
		if this.w + this.u_x > sw then this.u_x = sw - this.w end
		wpopups[#wpopups + 1] = this
	end

	local function gen_for_obj(obj)
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

		if obj then
			items[#items+1] = wchild(widget.text {u_text = obj.name or "*FIXME*"})
			addi("Examine", obj.examine or function ()
				cons_print("EX: Whoops, apparently obj.examine is nil.")
			end)
			if obj.actions then
				local _,t
				for _,t in pairs(obj.actions) do
					addi(t.text, t.fn)
				end
			end
		else
			items[#items+1] = wchild(widget.text {u_text = "Water"})
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

		return this
	end

	local obj = obj_get_top(cobjs)
	if obj.fail then obj = nil end
	local menu = gen_for_obj(obj)

	local itmenu = widget.box {
		layout = widget.layout.vbox_flow {},
		maxw = sw, maxh = sh,
	}

	local function addi2(obj)
		text = (obj and (obj.name or "*FIXME*")) or "Water"
		itmenu.add_child(widget.box {
			layout = widget.layout.hbox_flow {},
			children = { wchild(widget.drawobj {u_obj = obj} ),
				wchild(widget.text {u_text = text}) },
			ev_mouse_button = function(bx, by, ax, ay, button, state)
				if button == 0 and not state then
					do_pop(gen_for_obj(obj), ax, ay)
				end
			end})
	end

	local _,l,o,i
	for _,l in pairs({"wall","obj","floor"}) do
		for i=#cobjs[l],1,-1 do
			addi2(cobjs[l][i])
		end
	end
	if #cobjs.floor == 0 then
		addi2(nil)
	end

	local outer = widget.box {
		layout = widget.layout.hbox_flow {},
		children = { wchild(menu), wchild(itmenu) },
	}
	do_pop(outer, x, y)
end

function hook_render(sec_current, sec_delta)
	local x,y
	local mx, my, _, mb

	mx, my, _, _, mb = common.mouse_get()

	set_sec_beg(sec_current)

	local sw, sh = common.img_get_dims(nil)
	local csx, csy = cam_x - math.floor(sw/2), cam_y - math.floor(sh/2)
	csx = math.max(0, math.min(16*#map_vis[1] - sw, csx))
	csy = math.max(0, math.min(16*#map_vis - sh, csy))
	local csxt, csyt = math.ceil(csx/16), math.ceil(csy/16)

	for y=csyt,math.min(csyt+13,#map_vis) do
	for x=csxt,math.min(csxt+20,#(map_vis[1])) do
		local tx, ty
		local typ = map_vis[y][x]
		local water = common.turf_get_gas(map, x, y, GAS.WATER)
		
		local cobjs = map_tiles[y][x]
		water = math.max(0, math.min(1, water))
		water = math.floor(water * 16 - 0.5)

		local _,l,o
		for _,l in pairs({"floor", "obj", "wall"}) do
			for _,o in pairs(cobjs[l]) do
				o.draw(timg, sec_current, sec_delta, (x-1)*16 - csx, (y-1)*16 - csy, map_vis, test_map)
			end

			if l == "floor" and water >= 0 then
				common.img_blit(img_tiles, (x-1)*16 - csx, (y-1)*16 - csy, BF_AM_THRES,
					16*water, 16*1, 16, 16, timg)
			end
		end
	end
	end

	local cx, cy
	cx = math.floor((mx+csx)/16)+1
	cy = math.floor((my+csy)/16)+1

	common.draw_rect_outl(nil,
		(cx-1)*16-csx, (cy-1)*16-csy,
		(cx-1)*16+15-csx, (cy-1)*16+15-csy,
		0xFF880000)

	wtiles.draw(2, 2)
	local _,wi
	for _,wi in pairs(wpopups) do
		wi.draw(wi.u_x, wi.u_y)
	end
end

mouse_drag = nil
tmr_atmos = nil
function hook_tick(sec_current, sec_delta)
	set_sec_beg(sec_current)

	tmr_atmos = tmr_atmos or timer_new(function(sec)
		common.map_tick_atmos(map)
	end, sec_current, 1.0/50.0)

	if mouse_drag then
		local mx, my = common.mouse_get()
		local dx, dy = mx - mouse_drag[1], my - mouse_drag[2]
		cam_x = cam_x - dx
		cam_y = cam_y - dy
		mouse_drag[1] = mx
		mouse_drag[2] = my
	end

	local i
	for i=1,#tick_list do
		local obj = tick_list[i]
		obj.tick(sec_current, sec_delta)
	end

	tmr_atmos(sec_current)
end

function hook_mouse(x, y, button, state)
	if button == 1 then
		if state then
			local mx, my = common.mouse_get()
			mouse_drag = {mx, my}
		else
			mouse_drag = nil
		end
	elseif button == 2 then
		if state then
			popup_clear()
			popup_do(x, y)
		end
	else
		if not state then
			local _,wi
			local nwpopups = wpopups
			wpopups = {}
			for _,wi in pairs(nwpopups) do
				widget_mouse_button(wi, wi.u_x, wi.u_y, x, y, button, state)
			end
		end
	end
end

print(testlua("this", 3.14, "meow"))
print(testlua("arsethis", 3.14, "meow"))


