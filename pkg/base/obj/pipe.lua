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

PIPE = {
	PIPE = 1,
	VENT = 2,
	T_WATER_IN = 3,
	T_AIR_OUT = 4,
}

function pnet_new(cfg)
	local this = {
		pipes = {},
		gases = {0, 0, 0, 0, 0},
	}; this.this = this

	function this.add(obj)
		if obj.pnet then obj.pnet.remove(obj) end
		obj.pnet = this
		table.insert(this.pipes, obj)
	end

	function this.remove(obj)
		obj.pnet = nil

		local i,o
		for i,o in pairs(this.pipes) do
			if o == obj then
				table.remove(this.pipes, i)
				break
			end
		end
	end

	function this.merge(pn2)
		while #pn2 > 0 do this.add(pn2[1]) end
	end

	return this
end

pnet_main = pnet_new {}

function pipe_new(cfg)
	local this = {
		x = cfg.x, y = cfg.y,
		layer = LAYER.FLOOR,
		link_pipe = true,

		subtype = cfg.subtype or PIPE.PIPE,

		pnet = nil,
	}; this.this = this

	pnet_main.add(this) -- FIXME: Use a flood fill and get this onto a network.

	function this.draw(sec_current, sec_delta, bx, by)
		local x, y = this.x, this.y
		local tx, ty = 0, 7
		local t0 = (x < #(map_vis[1])) and obj_has_any(map_tiles[y][x+1], "link_pipe")
		local t1 = (y < #map_vis) and obj_has_any(map_tiles[y+1][x], "link_pipe")
		local t2 = (x > 1) and obj_has_any(map_tiles[y][x-1], "link_pipe")
		local t3 = (y > 1) and obj_has_any(map_tiles[y-1][x], "link_pipe")
		
		if t0 then tx = tx + 1 end
		if t1 then tx = tx + 2 end
		if t2 then tx = tx + 4 end
		if t3 then tx = tx + 8 end

		common.img_blit(img_tiles, bx, by, BF_AM_THRES,
			16*tx, 16*ty, 16, 16)

		if this.subtype == PIPE.PIPE then
			-- yeah.
		elseif this.subtype == PIPE.VENT then
			tx, ty = 5, 0
			common.img_blit(img_tiles, bx, by, BF_AM_THRES,
				16*tx, 16*ty, 16, 16)
		elseif this.subtype == PIPE.T_WATER_IN then
			tx, ty = 3, 0
			common.img_blit(img_tiles, bx, by, BF_AM_THRES,
				16*tx, 16*ty, 16, 16)
		elseif this.subtype == PIPE.T_AIR_OUT then
			tx, ty = 4, 0
			common.img_blit(img_tiles, bx, by, BF_AM_THRES,
				16*tx, 16*ty, 16, 16)
		end
	end

	function this.examine()
		if this.subtype == PIPE.PIPE then
			cons_print("EX: That's a pipe.")
		elseif this.subtype == PIPE.VENT then
			cons_print("EX: That's a vent.")
			cons_print("Removes water, injects air, attracts monsters.")
		elseif this.subtype == PIPE.T_WATER_IN then
			cons_print("EX: That's a water tank.")
			cons_print("It sucks water into it.")
		elseif this.subtype == PIPE.T_AIR_OUT then
			cons_print("EX: That's an air tank.")
			cons_print("It outputs air.")
		else
			cons_print("EX: That's an undocumented piece of Atmosian equipment.")
			cons_print("Better not mess with it.")
		end
	end

	return this
end

