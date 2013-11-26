--[[
Something to get your knickers in a knot about.

Type this into your shell.
local b = "hi"; function a() { print(b) } a();

This works fine in the Squirrel shell.
However, this breaks horribly when you do it here.

If I can't get that working soon, I'm going to dump Squirrel and move back to Lua,
because this is just complete horseshit.

--GM

OK, porting to Lua because fuck Squirrel.

--GM

]]
BF_M_AMODE  = 0x00000003
BF_AM_DIRECT   = 0x00000000
BF_AM_THRES    = 0x00000001
BF_AM_BLEND    = 0x00000002
BF_AM_DITHER   = 0x00000003

img_tiles = common.img_load("pkg/base/gfx/hello.png")

local test_map = {
	"     ####           ",
	"     #..#           ",
	"     #..#           ",
	"  ####..#           ",
	"  #.....#           ",
	"  #######           ",
	"     #..####        ",
	"     #.$...#        ",
	"     #..####        ",
	"   ########         ",
	"   #......#         ",
	"   #......#         ",
	"   ########         ",
}

TURF = {
	WATER = 0,
	FLOOR = 1,
	WALL = 2,
}

GAS = {
	WATER = 0,

	O2 = 1,
	N2 = 2,
	CO2 = 3,
	CH4 = 4,
}

test_map_trn = {
	[" "] = TURF.WATER,
	["."] = TURF.FLOOR,
	["$"] = TURF.FLOOR,
	["#"] = TURF.WALL,
}

wall_list = {}

-- Lua is 1-based. Ugh. But we'll cope.
print(#(test_map[1]), #test_map)
map = common.map_new(#(test_map[1]), #test_map)

local x,y
for y=1,#test_map do
	local s = test_map[y]
	for x=1,#s do
		local c = s:sub(x,x)
		local ctype = test_map_trn[c]
		--print(x, y, c, ctype)
		common.turf_set_type(map, x, y, ctype)
		common.turf_reset_gas(map, x, y)

		if ctype == TURF.WALL then
			wall_list[#wall_list+1] = {x, y}
		end

		if c == "$" then
			drain_x = x
			drain_y = y
		end
	end
end


poop = 1
deadwall = math.floor(math.random() * #wall_list) + 1
local x,y
x = wall_list[deadwall][1]
y = wall_list[deadwall][2]
common.turf_set_type(map, x, y, TURF.FLOOR)

function hook_render(sec_current, sec_delta)
	common.img_blit(img_tiles, 0, 0, BF_AM_THRES)
	-- TODO
end

function hook_tick(sec_current, sec_delta)
	poop = poop - 1
	if poop <= 0 then
		poop = 100
		local x, y
		x = wall_list[deadwall][1]
		y = wall_list[deadwall][2]
		common.turf_set_type(map, x, y, TURF.WALL)
		common.turf_reset_gas(map, x, y)

		deadwall = math.floor(math.random() * #wall_list) + 1
		x = wall_list[deadwall][1]
		y = wall_list[deadwall][2]
		common.turf_set_type(map, x, y, TURF.FLOOR)
	end

	common.turf_set_gas(map, drain_x, drain_y, GAS.WATER,
		common.turf_get_gas(map, drain_x, drain_y, GAS.WATER) * 0.8)
end

