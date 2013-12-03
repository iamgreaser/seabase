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

function door_new(cfg)
	local this = {
		x = cfg.x, y = cfg.y,
		layer = LAYER.WALL,
		link_wall = true,
		name = "Door",

		tmr_openclose = nil,
		openness = 0,
		open_state = false,
		opening = false,
		closing = false,
		
		u_speed = cfg.u_speed or 1.0,
		water_lock = cfg.water_lock or false,
	}

	this.this = this

	if this.water_lock then
		this.water_thres = 0.0
		if cfg.open ~= false then
			this.open_state = true
			this.openness = 7
		end
	end

	if cfg.open == true then
		this.open_state = true
		this.openness = 7
	end

	--map_vis[this.y][this.x] = TURF.FLOOR

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
		if this.water_lock then
			local t0 = common.turf_get_type(map, this.x - 1, this.y)
			local t1 = common.turf_get_type(map, this.x, this.y - 1)
			local t2 = common.turf_get_type(map, this.x + 1, this.y)
			local t3 = common.turf_get_type(map, this.x, this.y + 1)

			local w0 = common.turf_get_gas(map, this.x - 1, this.y, GAS.WATER)
			local w1 = common.turf_get_gas(map, this.x, this.y - 1, GAS.WATER)
			local w2 = common.turf_get_gas(map, this.x + 1, this.y, GAS.WATER)
			local w3 = common.turf_get_gas(map, this.x, this.y + 1, GAS.WATER)

			w0 = ((t0 == TURF.WALL and 0) or w0)
			w1 = ((t1 == TURF.WALL and 0) or w1)
			w2 = ((t2 == TURF.WALL and 0) or w2)
			w3 = ((t3 == TURF.WALL and 0) or w3)

			local wn = math.max(w0, w1, w2, w3)

			if wn > 0.03 and this.open_state and not this.water_closed then
				this.water_closed = true
				this.close()
			end

			if this.water_closed then
				if wn < 0.03 and (not this.closing) then
					this.water_closed = false
					this.open()
				end
			end
		end
		if this.tmr_openclose then
			this.tmr_openclose(sec_current, sec_delta)
		end
	end

	function this.open(sec_current)
		if (not this.open_state) or (this.closing) then
			this.closing = false
			this.opening = true
			this.tmr_openclose = timer_new(this.f_open, sec_current, this.u_speed/8.0)
		end
	end

	function this.close(sec_current)
		if (this.open_state) or (this.opening) then
			this.opening = false
			this.closing = true
			this.tmr_openclose = timer_new(this.f_close, sec_current, this.u_speed/8.0)
		end
	end

	function this.draw(img, sec_current, sec_delta, bx, by)
		local tx, ty = this.openness, 3
		if this.water_lock then ty = ty + 1 end
		common.img_blit(img_tiles, bx, by, BF_AM_THRES,
			16*tx, 16*ty, 16, 16, img)
	end

	function this.examine()
		cons_print("EX: That's a door.")
		cons_print("It opens and closes.")
	end

	this.actions = {
		{text = "Open", fn = this.open},
		{text = "Close", fn = this.close},
	}

	return this
end

