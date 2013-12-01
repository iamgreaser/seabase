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

function puts(x, y, s, img)
	local i
	for i=1,#s do
		local c = s:byte(i) - 0x20
		common.img_blit(img_font, x, y, BF_AM_THRES, c*6, 0, 6, 8, img)
		x = x + 6
	end
end

function cons_print(...)
	-- TODO: log this to an on-screen console OSLT
	print(...)
end

widget = {}
widget.layout = {}

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

	this.draw_sub(img, bx, by, bw, bh, ax, ay):
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
	
	this.add_children_batch(cfg):
		Adds several children to this widget.
		Everything that has children SHOULD call this before returning!
		Just use the cfg you were given to create "this".
	
	Functions to optionally override:

	this.on_add_child(child, cfg):
		Adds a child to this widget.

		Default just adds the child to the layout when it's not nil.

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

	caught = this.on_mouse_button(bx, by, ax, ay, button, state):
		Indicates a mouse button press/release.
		Returns true if the click was caught.

		For transferring to children at (sx, sy), do this:
			child.on_mouse_button(bx - sx, by - sy, ax, ay, button, state)

		The default method calls on_mouse_button on its layout (if any exists),
		and if that fails then it calls ev_mouse_button if it exists.
		Failing THAT, it returns false.

	Functions to override:

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
		ev_mouse_button = cfg.ev_mouse_button,
	}; this.this = this

	if this.w then
		this.w = math.min(math.max(this.w, this.minw or this.w), this.maxw or this.w)
	end
	if this.h then
		this.h = math.min(math.max(this.h, this.minh or this.h), this.maxh or this.h)
	end

	function this.resize(w, h)
		if this.w == w and this.h == h then return end

		this.w, this.h = w, h
		return this.on_resize(w, h)
	end

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
		minw = minw or this.minw
		minh = minh or this.minh
		maxw = maxw or this.maxw
		maxh = maxh or this.maxh

		if this.minw and this.minw > minw then this.minw = minw end
		if this.minh and this.minh > minh then this.minh = minh end
		if this.maxw and this.maxw < maxw then this.maxw = maxw end
		if this.maxh and this.maxh < maxh then this.maxh = maxh end

		local w, h = this.on_pack(minw, minh, maxw, maxh, expand, ...)
		-- TODO: sanity checks + image nabbing
		this.w = w or 1
		this.h = h or 1
		return w, h
	end

	function this.get_img(w, h)
		return nil -- TODO!
	end

	function this.add_child(child, cfg)
		child.set_parent(this)
		this.on_add_child(child, cfg)
	end

	function this.add_children_batch(cfg)
		if not cfg.children then return end

		local i, v
		for i,v in pairs(cfg.children) do
			this.add_child(v.child, v.cfg)
		end
	end

	-- Stuff to override
	function this.on_mouse_button(...)
		return (this.layout and this.layout.on_mouse_button(...))
			or (this.ev_mouse_button and this.ev_mouse_button(...))
	end

	function this.on_pack(minw, minh, maxw, maxh, expand, ...)
		-- Sane default
		local w, h 

		if expand then
			w, h = this.get_max_dims()
		else
			w, h = this.get_nom_dims()
		end

		w = math.max(1, w or 1)
		h = math.max(1, h or 1)

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

	function this.on_draw(img, bx, by, bw, bh, ax, ay, ...)
		-- override me!
	end

	function this.on_resize(w, h, ...)
		-- override me!
	end

	function this.on_add_child(child, cfg)
		if this.layout then
			this.layout.add_child(child, cfg)
		end
	end

	function this.is_safe_clip(w, h, ...)
		return true
	end

	return this
end

--[[
	Horizontally-biased box flow layout.
]]
function widget.layout.hbox_flow(cfg)
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
				nw = x
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
		this.make_child_layout(nil, nil, maxw, maxh, expand)
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

--[[
	Vertically-biased box flow layout.
]]
function widget.layout.vbox_flow(cfg)
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

--[[
	Free layout.
]]
function widget.layout.free(cfg)
	local this = widget_new(cfg)
	this.children = {}

	function this.on_add_child(child, cfg)
		this.children[#this.children + 1] = {
			child = child, 
			x = cfg.x, y = cfg.y,
		}
	end

	local s_on_pack = this.on_pack
	function this.on_pack(minw, minh, maxw, maxh, expand, ...)
		local _,cs
		for _,cs in pairs(this.children) do
			cs.child.pack_sub(nil, nil, nil, nil, expand, ...)
		end
		return s_on_pack(minw, minh, maxw, maxh, expand, ...)
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

widget.layout.default = widget.layout.hbox_flow

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

		layout = widget.layout.free {},

		w = w, h = h,
	}
end

--[[
	Box widget.

	Creates a box.
]]
function widget.box(cfg)
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

	this.add_children_batch(cfg)

	return this
end

--[[
	Text widget.

	Displays text.
]]
function widget.text(cfg)
	local this = widget_new(cfg)

	this.u_text = cfg.u_text

	function this.on_pack(minw, minh, maxw, maxh, expand)
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

--[[
	Helper function to make UI definitions not look silly.
]]
function wchild(child, cfg)
	return { cfg = cfg or {}, child = child }
end

--[[
	Function to simulate a click on a widget.
]]

function widget_mouse_button(wi, wx, wy, ax, ay, button, state, ...)
	local w, h = wi.get_nom_dims()
	return ax >= wx and ax < wx + w
		and ay >= wy and ay < wy + h
		and wi.on_mouse_button(ax - wx, ay - wy, ax, ay, button, state, ...)
end


