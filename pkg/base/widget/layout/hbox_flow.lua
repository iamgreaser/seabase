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
	Horizontally-biased box flow layout.
]]

do
	local cfg = ...
	local this = widget_new(cfg)
	this.children = {}

	function this.on_add_child(child, cfg)
		this.children[#this.children + 1] = {
			child = child, 
			x = 0, y = 0,
			w = 0, h = 0,
		}
	end

	function this.make_child_layout(minw, minh, maxw, maxh, expand)
		local _,child
		local nw,nh = 0,0
		local x,y = 0,0
		local next_y = 0
		local lminw, lminh = this.get_min_dims()

		for _,cs in pairs(this.children) do
			local child = cs.child
			cs.w, cs.h = child.pack_sub(nil, nil, maxw, maxh, expand)
			if maxw and cs.w + x + 1 > maxw then
				cs.w = math.min(cs.w, maxw)
				nw = math.max(nw, math.min(cs.w, maxw))
				y = y + next_y + 1
				next_y = cs.h
				cs.x = 0
				cs.y = y
				x = cs.w
			else
				cs.x = x + 1
				if x == 0 then cs.x = 0 end
				cs.y = y
				next_y = math.max(next_y, cs.h)
				x = cs.x + cs.w
				nw = math.max(nw, x)
			end
			nh = math.max(nh, math.min(maxh or (y + cs.h), y + cs.h))
			child.resize(cs.w, cs.h)
		end

		--print("HONF", nw, nh)
		return nw, nh
	end

	function this.on_mouse_button(bx, by, ax, ay, button, state, ...)
		for _,cs in pairs(this.children) do
			local child = cs.child
			local in_range = (bx >= cs.x and bx < cs.x + cs.w
				and by >= cs.y and by < cs.y + cs.h)

			if in_range and child.on_mouse_button(bx - cs.x, by - cs.y,
					ax, ay, button, state, ...) then
				return true
			end
		end

		return this.ev_mouse_button and this.ev_mouse_button(
			bx, by, ax, ay, button, state, ...)
	end

	function this.on_resize(maxw, maxh)
		--this.make_child_layout(nil, nil, maxw, maxh, nil)
	end

	function this.on_pack(minw, minh, maxw, maxh, expand)
		return this.make_child_layout(nil, nil, maxw, maxh, expand)
	end

	function this.on_draw(img, bx, by, bw, bh, ax, ay, ...)
		local _,child

		for _,cs in pairs(this.children) do
			local child = cs.child
			local sx, sy = cs.x, cs.y
			child.draw_sub(img, sx + bx, sy + by,
				cs.w, cs.h,
				ax + sx, ay + sy, ...)
		end
	end

	return this
end

