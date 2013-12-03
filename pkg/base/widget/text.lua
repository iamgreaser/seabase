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
	Text widget.

	Displays text.
]]

do
	local cfg = ...
	local this = widget_new(cfg)

	this.u_text = cfg.u_text

	function this.on_pack(maxw, maxh, expand)
		local w, h = 0, 1
		-- TODO: handle newlines

		w = #this.u_text

		return w*6, h*8
	end

	function this.on_draw(img, bx, by, bw, bh, ax, ay, ...)
		-- TODO: set this up so it doesn't overflow the text
		puts(bx, by, this.u_text, img)
	end

	this.add_children_batch(cfg)

	return this
end

