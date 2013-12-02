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
	Tile object widget.

	Creates a surface to draw the image for a given tile.
]]

do
	local cfg = ...
	if cfg.u_obj then
		cfg.w, cfg.h = 16, 16
	else
		cfg.w, cfg.h = 1, 1
	end
	local this = widget_new(cfg)
	this.u_obj = cfg.u_obj

	function this.on_pack(minw, minh, maxw, maxh, expand)
		--print(minw, minh, maxw, maxh)
		if this.u_obj then return 16, 16
		else return 1, 1 end
	end

	function this.on_draw(img, bx, by, bw, bh, ax, ay, ...)
		-- TODO: sort out the APIs and whatnot
		if this.u_obj then
			this.u_obj.draw(nil, nil, ax, ay)
		end
	end

	return this
end

