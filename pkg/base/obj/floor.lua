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

function floor_new(cfg)
	local this = {
		x = cfg.x, y = cfg.y,
		layer = LAYER.FLOOR,
		name = "Floor",

		has_tiles = true,
	}; this.this = this

	if cfg.has_tiles ~= nil then this.has_tiles = cfg.has_tiles end

	function this.draw(img, sec_current, sec_delta, bx, by)
		local tx, ty = 2, 0
		if this.has_tiles then tx = 1 end
		common.img_blit(img_tiles, bx, by, BF_AM_THRES,
			16*tx, 16*ty, 16, 16, img)
	end

	function this.examine()
		cons_print("EX: That's a floor.")
	end

	return this
end


