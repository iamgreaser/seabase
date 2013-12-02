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
	Vertically-biased box flow layout.
]]
do
	local cfg = ...
	local this = widget.layout.hbox_flow(cfg)

	-- TECHNICALLY THIS IS A MIXIN NOT A CLASS ABSTRACTION SO IT'S OK
	function this.make_child_layout(minw, minh, maxw, maxh, expand)
		local _,child
		local nw,nh = 0,0
		local x,y = 0,0
		local next_x = 0
		local lminw, lminh = this.get_min_dims()

		for _,cs in pairs(this.children) do
			local child = cs.child
			cs.w, cs.h = child.pack_sub(nil, nil, maxw, maxh, expand)
			if maxh and cs.h + y + 1 > maxh then
				cs.h = math.min(cs.h, maxh)
				nh = math.max(nh, math.min(cs.h, maxh))
				x = x + next_x + 1
				next_x = cs.w
				cs.y = 0
				cs.x = x
				y = cs.h
			else
				cs.y = y + 1
				if y == 0 then cs.y = 0 end
				cs.x = x
				next_x = math.max(next_x, cs.w)
				y = cs.y + cs.h
				nh = y
			end
			nw = math.max(nw, math.min(maxw or (x + cs.w), x + cs.w))
			child.resize(cs.h, cs.w)
		end

		--print("HONF", nw, nh)
		return nw, nh
	end

	return this
end


