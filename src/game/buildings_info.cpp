#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <game/buildings_info.h>

t_BuildingsInfo aBuildingsInfo[] = {
	{
		"Reactor",
		"Most important building,\n" \
		"needed to build other\n" \
		"things. Supplies your base\n" \
		"with energy. There can be\n" \
		"only one reactor at the\n" \
		"same time.",
		0,
		200,
		86.0f,
		110.0f
	},

	{
		"Spawn",
		"Allows your teammates to\n" \
		"spawn at this building. If\n" \
		"there are no spawns and no\n" \
		"players left, your team has\n" \
		"lost. So handle these carefully!",
		10,
		70,
		78.0f,
		125.0f
	},

	{
		"Ammo Disp. Shotgun",
		"Supplies your team with the\n" \
		"shotgun and ammunition.",
		8,
		60,
		62.0f,
		62.0f
	},

	{
		"Health Station",
		"Refreshs your health in case\n" \
		"of medical emergencies.",
		10,
		50,
		55.0f,
		55.0f
	},

	{
		"Repeater",
		"Mini Reactor. Allows you to\n" \
		"build at any other spot on\n" \
		"the map, as long as there is\n" \
		"a working reactor.",
		12,
		50,
		64.0f,
		70.0f
	},

	{
		"Gun Turret",
		"Shoots at enemies\n" \
		"automatically. High range\n" \
		"and fire rate, but low\n" \
		"damage. Be careful: Might\n" \
		"hurt teammates as well!",
		10,
		40,
		40.0f,
		93.0f,
		700,
		12
	},

	{
		"Shield Generator",
		"Protects friendly players and\n" \
		"buildings from projectiles\n" \
		"by tossing them back to their\n" \
		"senders.",
		12,
		40,
		80.0f,
		72.0f
	},

	{
		"Ammo Disp. Grenade",
		"Supplies your team with the\n" \
		"grenade launcher and ammo.",
		8,
		60,
		62.0f,
		62.0f
	},

	{
		"Teeleporter",
		"Makes faster-than-light\n" \
		"travelling between two of\n" \
		"those finally possible.",
		10,
		50,
		96.0f,
		25.0f
	},

	{
		"Armor Station",
		"Gives you protective armor\n" \
		"against weapons of all kinds.",
		10,
		50,
		55.0f,
		55.0f
	},

	{
		"Ammo Disp. Laser",
		"Supplies your team with\n" \
		"laser weapons and\n" \
		"ammunition. Not affected\n" \
		"by shields!",
		8,
		60,
		62.0f,
		62.0f
	},

	{
		"Shotgun Turret",
		"Shoots at enemies\n" \
		"automatically using\n" \
		"a shotgun. High damage,\n" \
		"but low range. Be\n" \
		"careful: Might hurt\n" \
		"teammates as well!",
		12,
		60,
		40.0f,
		93.0f,
		500,
		30
	}
};