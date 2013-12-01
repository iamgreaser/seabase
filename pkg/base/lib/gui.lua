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

widget = {}
widget.layouts = {}


--[[
	Base class all widgets abstract from.

	NOTE: maximum is more important than minimum.
	NOTE: Do not call the on_* stuff directly. Use the main API.

	You can put these into cfg:

	w, h: Dimensions of this widget. (nil = use nom/min/max as below, otherwise set all to this by default)
	nomw, nomh: Nominal dimensions of this widget. (nil == use minimum)
	minw, minh: Minimum dimensions of this widget. (nil == no minimum)
	maxw, maxh: Maximum dimensions of this widget. (nil == no maximum)
	cbg, cborder, ctext: Theme colours (set to nil to inherit from parent).
	layout: Layout manager to use (usually safe to set to nil).
	children: Children to automatically add using this.add_child(), in this format:
		children[i].cfg = {}
		children[i].child = widget.somethingrather {}
	u_*: Widget-specific stuff ("user").

	Functions to typically leave as-is:
	w, h = this.get_min_dims():
	w, h = this.get_max_dims():
	w, h = this.get_nom_dims():
		Get minimum, maximum, and nominative dimensions respectively.
		These can return nil meaning "no limit".
		For get_nom_dims: If any dimension is nil, return its minimum instead.
	
	val = this.inherit(name):
		Gets the first non-nil of variable "name" up the family tree.
		Typically used for theme stuff.

	w, h = this.pack():
		Calculates this widget's and its childrens' sizes and positions.
		Returns width and height.

	w, h = this.pack_sub(minw, minh, maxw, maxh, expand):
		Calls this.on_pack() and this.resize().
		Does some sanity checks, too.

	this.draw(ax, ay):
		Draws this widget and its children at ax, ay.
		Note, if you're drawing children, use draw_sub.

	this.draw_sub(parent, img, bx, by, bw, bh, ax, ay):
		Sanely clips the region, gets an image if need be, then calls this.on_draw().

	this.get_img(w, h):
		Gets an image for rendering a w by h rectangle.
		Returns nil if one is not necessary.
		Calls this.is_safe_clip() and this.resize() internally.

	this.resize(w, h):
		Resize this widget.
		Calls this.on_resize() and may invalidate img.
	
	this.set_parent(parent):
		Sets this widget's parent.
		Called automatically by this.add_child.
	
	this.add_child(child, cfg):
		Adds a child to this widget.
		Calls child.set_parent().
		Note, cfg pertains to the child's positioning and stuff,
		and depends on what layout you are using and whatnot.
	
	Functions to optionally override:

	this.on_add_child(child, cfg):
		Adds a child to this widget.

		Default just does nothing.

	w, h = this.on_pack(minw, minh, maxw, maxh, expand):
		Resize this widget to suit.
		Any of these arguments may be nil.
		Return your new width and height.

		"expand" is a boolean.
		If it is false, you should aim for the minimum dimensions.
		If it is true, you should aim for the maximum dimensions.

		this.resize() is called by this.pack().

	result = this.is_safe_clip(w, h):
		Returns true if this widget can clip safely.
		Default always returns true.

	this.on_resize(w, h):
		Stuff to do when this gets resized.
		If you don't need to do anything special when this actually happens,
		you don't need to change this.

		this.w and this.h are already set for you by this point.

	Functions to override:

	this.on_mouse_button(parent, bx, by, ax, ay, button, state):
		Indicates a mouse button press/release.
		
	this.on_draw(parent, img, bx, by, bw, bh, ax, ay):
		Draws on the image img,
		at top-left corner bx,by,
		with dimensions bw x bh.

		You are being drawn by the parent widget "parent".

		Absolute screen coordinates ax,ay may be useful wrt dithering.

		For drawing children at (sx, sy), do this:
			child.draw_sub(img, sx, sy, bw, bh, ax + sx - bx, ay + sy - by)
]]
function widget_new(cfg)
	local this = {
		w = cfg.w or cfg.nomw or cfg.minw, h = cfg.h or cfg.nomh or cfg.minh,
		minw = cfg.minw or cfg.w, minh = cfg.minh or cfg.h,
		maxw = cfg.maxw or cfg.w, maxh = cfg.maxh or cfg.h,
		layout = cfg.layout,
		cbg = cfg.cbg, cborder = cfg.cborder, ctext = cfg.ctext,
		parent = nil,
	}; this.this = this

	this.w = math.min(math.max(this.w, this.minw or this.w), this.maxw or this.w)
	this.h = math.min(math.max(this.h, this.minh or this.h), this.maxh or this.h)

	function this.get_min_dims()
		return this.minw, this.minh
	end

	function this.get_max_dims()
		return this.maxw, this.maxh
	end

	function this.get_nom_dims()
		return this.w or this.minw, this.h or this.minh
	end

	function this.set_parent(parent)
		this.parent = parent
	end

	function this.inherit(name)
		if this[name] ~= nil then
			return this[name]
		elseif this ~= widget.root then
			return (this.parent or widget.root).inherit(name)
		else
			return nil
		end
	end

	function this.draw(ax, ay, ...)
		return this.on_draw(nil, ax, ay, this.w, this.h, ax, ay, ...)
	end

	function this.draw_sub(img, bx, by, bw, bh, ax, ay, ...)
		-- TODO: sanity checks
		return this.on_draw(this.get_img() or img,
			bx, by, bw, bh, ax, ay, ...)
	end

	function this.pack()
		return this.pack_sub(nil, nil, nil, nil, false)
	end

	function this.pack_sub(minw, minh, maxw, maxh, expand, ...)
		local w, h = this.on_pack(minw, minh, maxw, maxh, expand, ...)
		-- TODO: sanity checks + image nabbing
		this.resize(w, h)
		return w, h
	end

	function this.add_child(child, cfg)
		child.set_parent(this)
		this.on_add_child()
	end

	-- Stuff to override
	function this.on_pack(minw, minh, maxw, maxh, expand, ...)
		-- Sane default
		local w, h 

		if expand then
			w, h = this.get_max_dims()
		else
			w, h = this.get_nom_dims()
		end

		if this.minw and w < this.minw then w = this.minw end
		if this.minh and h < this.minh then h = this.minh end
		if this.maxw and w > this.maxw then w = this.maxw end
		if this.maxh and h > this.maxh then h = this.maxh end

		if minw and w < minw then w = minw end
		if minh and h < minh then h = minh end
		if maxw and w > maxw then w = maxw end
		if maxh and h > maxh then h = maxh end

		return w, h
	end

	function this.on_draw(parent, img, bx, by, bw, bh, ax, ay, ...)
		-- override me!
	end

	function this.on_resize(w, h, ...)
		-- override me!
	end

	function this.is_safe_clip(w, h, ...)
		return true
	end

	return this
end

--[[
	Root widget.

	Used for themes and whatnot.
]]
do
	local w, h = common.img_get_dims(nil)
	widget.root = widget_new {
		cbg = 0xFF666666,
		cborder = 0xFF999999,
		ctext = 0xFFFFFFFF, -- not like anything else is supported right now

		w = w, h = h,
	}
end

--[[
	Box widget.

	Creates a box.
]]
function widget.box(cfg)
	local this = widget_new(cfg)
	this.layout = cfg.layout or widget.layouts.default {}
	this.hpad = cfg.hpad or 1
	this.vpad = cfg.vpad or 1

	this.minw = cfg.minw or 1
	this.minh = cfg.minh or 1

	function this.resize(w, h)
		this.w, this.h = w-2-this.hpad*2, h-2-this.vpad*2
		this.on_resize(this.w, this.h)
	end

	function this.on_draw(img, bx, by, bw, bh, ax, ay)
		common.draw_rect_fill(img, bx, by, bx+bw-1, by+bh-1,
			this.inherit("cbg"))
		common.draw_rect_outl(img, bx, by, bx+bw-1, by+bh-1,
			this.inherit("cborder"))
	end

	return this
end

--[[
	Horizontally-biased box flow layout.
]]
function widget.layouts.hbox_flow(cfg)
	local this = {
		children = cfg.children or {},
	}; this.this = this

	return this
end

widget.layouts.default = widget.layouts.hbox_flow
