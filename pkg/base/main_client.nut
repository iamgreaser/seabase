print("squirrel test");

local test_map = [
	"                    ",
	"                    ",
	"                    ",
	"     ####           ",
	"     #..#           ",
	"     #..#           ",
	"  ####..#           ",
	"  #.....#           ",
	"  ####..#           ",
	"     #..####        ",
	"     #.....#        ",
	"     #..####        ",
	"   ###..###         ",
	"   #......#         ",
	"   #......#         ",
	"   ########         ",
	"                    ",
	"                    ",
	"                    ",
	"                    ",
]

local TURF = {
	WATER = 0,
	FLOOR = 1,
	WALL = 2,
}

local test_map_trn = {
	[" "] = TURF.WATER,
	["."] = TURF.FLOOR,
	["#"] = TURF.WALL,
}

// NOTE: Squirrel is 0-based like a real language.
local map = map_new(test_map.len(), test_map[0].len());

local x,y
foreach(y,s in test_map)
{
	foreach(x,c in s)
	{
		local ctype = test_map_trn[c.tochar()]
		turf_set_type(map, x, y, ctype);
		//turf_reset_gas(x, y);
	}
}

