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
	Box widget.

	Creates a box.
]]

do
	local cfg = ...
	local this = widget_new(cfg)
	this.layout = cfg.layout or widget.layout.default {}
	this.hpad = cfg.hpad or 1
	this.vpad = cfg.vpad or 1

	this.minw = cfg.minw or 1
	this.minh = cfg.minh or 1

	function this.on_pack(minw, minh, maxw, maxh, expand)
		local pw, ph = this.hpad*2+2, this.vpad*2+2
		local w, h = this.layout.pack_sub(minw, minh, maxw, maxh, expand)
		local nw, nh = this.get_nom_dims()

		w = w + pw
		h = h + ph
		if w < nw then w = nw end
		if h < nh then h = nh end

		return w, h
	end

	function this.on_resize(w, h)
		local pw, ph = this.hpad*2+2, this.vpad*2+2
		this.layout.resize(w - pw, h - ph)
	end

	function this.on_draw(img, bx, by, bw, bh, ax, ay, ...)
		common.draw_rect_fill(img, bx, by, bx+bw-1, by+bh-1,
			this.inherit("cbg"))
		common.draw_rect_outl(img, bx, by, bx+bw-1, by+bh-1,
			this.inherit("cborder"))
		this.layout.draw_sub(img, bx + this.hpad + 1, by + this.vpad + 1,
			bw - this.hpad*2 - 2, bh - this.vpad*2 - 2,
			ax + this.hpad + 1, ay + this.vpad + 1, ...)
	end

	function this.on_mouse_button(bx, by, ...)
		local ox, oy = this.hpad + 1, this.vpad + 1
		return (this.layout and this.layout.on_mouse_button(bx - ox, by - oy, ...))
			or (this.ev_mouse_button and this.ev_mouse_button(bx, by, ...))
	end

	this.add_children_batch(cfg)

	return this
end


