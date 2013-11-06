/*
Something to get your knickers in a knot about.

Type this into your shell.
local b = "hi"; function a() { print(b) } a();

This works fine in the Squirrel shell.
However, this breaks horribly when you do it here.

If I can't get that working soon, I'm going to dump Squirrel and move back to Lua,
because this is just complete horseshit.

--GM
*/

local test_map = [
	"                    ",
	"                    ",
	"                    ",
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
	"                    ",
	"                    ",
	"                    ",
	"                    ",
];

TURF <- {
	WATER = 0,
	FLOOR = 1,
	WALL = 2,
};

GAS <- {
	WATER = 0,

	O2 = 1,
	N2 = 2,
	CO2 = 3,
	CH4 = 4,
};

local test_map_trn = {
	[" "] = TURF.WATER,
	["."] = TURF.FLOOR,
	["$"] = TURF.FLOOR,
	["#"] = TURF.WALL,
};

wall_list <- [];

// NOTE: Squirrel is 0-based like a real language.
map <- map_new(test_map.len(), test_map[0].len());

local x,y;
foreach(y,s in test_map)
{
	foreach(x,c in s)
	{
		local ctype = test_map_trn[c.tochar()];
		turf_set_type(map, x, y, ctype);
		turf_reset_gas(map, x, y);

		if(ctype == TURF.WALL)
		{
			wall_list.append([x, y]);
		}

		if(c.tochar() == "$")
		{
			drain_x <- x;
			drain_y <- y;
		}
	}
}


poop <- 1;
deadwall <- rand() % wall_list.len();
local x,y;
x = wall_list[deadwall][0];
y = wall_list[deadwall][1];
turf_set_type(map, x, y, TURF.FLOOR);

function hook_tick(sec_current, sec_delta)
{
	poop--;
	if(poop <= 0)
	{
		poop = 100;
		local x, y;
		x = wall_list[deadwall][0];
		y = wall_list[deadwall][1];
		turf_set_type(map, x, y, TURF.WALL);
		turf_reset_gas(map, x, y);

		deadwall <- rand() % wall_list.len();
		x = wall_list[deadwall][0];
		y = wall_list[deadwall][1];
		turf_set_type(map, x, y, TURF.FLOOR);
	}

	turf_set_gas(map, drain_x, drain_y, GAS.WATER,
		turf_get_gas(map, drain_x, drain_y, GAS.WATER) * 0.8);
}

