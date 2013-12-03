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
	Free layout.
]]

do
	local cfg = ...
	local this = widget_new(cfg)
	this.children = {}

	function this.on_add_child(child, cfg)
		this.children[#this.children + 1] = {
			child = child, 
			x = cfg.x, y = cfg.y,
		}
	end

	local s_on_pack = this.on_pack
	function this.on_pack(maxw, maxh, expand, ...)
		local _,cs
		for _,cs in pairs(this.children) do
			cs.child.pack_sub(nil, nil, expand, ...)
		end
		return s_on_pack(maxw, maxh, expand, ...)
	end

	function this.on_draw(img, bx, by, bw, bh, ax, ay, ...)
		local _,cs
		for _,cs in pairs(this.children) do
			cs.child.draw_sub(img, bx + cs.x, by + cs.y,
				bw - cs.x, bh - cs.y,
				ax + cs.x, ay + cs.y, ...)
		end
	end
end

