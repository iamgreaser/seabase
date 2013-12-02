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

		tmr_openclose = nil,
		openness = 0,
		open_state = false,
		opening = false,
		closing = false,
	}

	this.this = this

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

