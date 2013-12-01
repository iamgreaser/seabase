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

function puts(x, y, s)
	local i
	for i=1,#s do
		local c = s:byte(i) - 0x20
		common.img_blit(img_font, x, y, BF_AM_THRES, c*6, 0, 6, 8)
		x = x + 6
	end
end

wtheme = {
	bg = 0xFF666666,
	border = 0xFF999999,
	text = 0xFFFFFFFF, -- not like anything else is supported right now
}

widget = {}
widget.layouts = {}

--[[
	Horizontally-biased box flow layout.
]]
function widget.layouts.hbox_flow(cfg)
	local this = {
		--
	}; this.this = this

	return this
end

widget.layouts.default = widget.layouts.hbox_flow

--[[
	Base class all widgets abstract from.

	x, y: Top-left corner of this widget relative to its parent.
	w, h: Nominal dimensions of this widget. (nil == use minimum)
	minw, minh: Minimum dimensions of this widget. (nil == no minimum)
	maxw, maxh: Maximum dimensions of this widget. (nil == no maximum)
	cbg, cborder, ctext: Theme colours (set to nil to inherit from parent).
	children: List of child widgets to use.
	layout: Layout manager to use (nil == use widget.layouts.hbox_flow).
]]
function widget_new(cfg)
	local this = {
		x = cfg.x, y = cfg.y,
		w = cfg.w, h = cfg.h,
		minw = cfg.minw or cfg.w, minh = cfg.minh or cfg.h,
		maxw = cfg.maxw or cfg.w, maxh = cfg.maxh or cfg.h,
		children = cfg.children or {},
		layout = cfg.layout or (widget.layouts.default {}),
		cbg = cfg.cbg, cborder = cfg.cborder, ctext = cfg.ctext,
		img = nil,
	}; this.this = this

	-- shifted these out because i'm getting a C stack overflow D:
	-- TODO: FIX THIS.

	this.maxw = math.max(this.minw, this.maxw)
	this.maxh = math.max(this.minh, this.maxh)
	this.w = math.min(this.w, this.maxw)
	this.h = math.min(this.h, this.maxh)

	return this
end

