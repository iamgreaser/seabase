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

function table_new(cfg)
	local this = {
		x = cfg.x, y = cfg.y,
		layer = LAYER.OBJ,
		name = "Table",
		link_table = true,
	}

	this.this = this

	function this.draw(sec_current, sec_delta, bx, by)
		local x, y = this.x, this.y
		local tx, ty = 0, 6
		local t0 = (x < #(map_vis[1])) and obj_get_top(map_tiles[y][x+1]).link_table
		local t1 = (y < #map_vis) and obj_get_top(map_tiles[y+1][x]).link_table
		local t2 = (x > 1) and obj_get_top(map_tiles[y][x-1]).link_table
		local t3 = (y > 1) and obj_get_top(map_tiles[y-1][x]).link_table
		
		if t0 then tx = tx + 1 end
		if t1 then tx = tx + 2 end
		if t2 then tx = tx + 4 end
		if t3 then tx = tx + 8 end

		common.img_blit(img_tiles, bx, by, BF_AM_THRES,
			16*tx, 16*ty, 16, 16)
	end

	function this.examine()
		cons_print("EX: That's a table.")
	end

	this.actions = {
		{text = "Delete", fn = function ()
			local cobjs = map_tiles[this.y][this.x]
			local i,o
			for i,o in pairs(cobjs.obj) do
				if o == this then
					table.remove(cobjs.obj, i)
					break
				end
			end
			if #cobjs.wall == 0 and #cobjs.obj == 0 then
				if #cobjs.floor == 0 then
					common.turf_set_type(map, this.x, this.y, TURF.WATER)
					map_vis[this.y][this.x] = TURF.WATER
				else
					common.turf_set_type(map, this.x, this.y, TURF.FLOOR)
					map_vis[this.y][this.x] = TURF.FLOOR
				end
			end
		end},
	}

	return this
end

