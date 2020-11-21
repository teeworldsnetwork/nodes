#ifndef GAME_BUILDINGS_INFO_H
#define GAME_BUILDINGS_INFO_H

#include <base/vmath.h>
#define MAX_INT 2147483647
#define MAX_BUILDINGS 512

enum
{
	B_REACTOR = 0, // 0
	B_SPAWN, // 1
	B_AMMO_SHOTGUN, // 2
	B_HEALTH, // 3

	B_REPEATER, // 4
	B_TURRET_GUN, // 5
	B_SHIELD, // 6
	B_AMMO_GRENADE, // 7

	B_TELEPORT, // 8
	B_ARMOR, // 9
	B_AMMO_LASER, // 10
	B_TURRET_SHOTGUN // 11
};

enum
{
	EFFECT_FALLOUT = 0,
	EFFECT_HEALING,
	EFFECT_RAMPAGE,
	EFFECT_FREEZE,
	EFFECT_BOMB
};

enum
{
	CRATE_HEALTH = 1,
	CRATE_ARMOR,
	CRATE_CONSTRUCTION,
	CRATE_BASEBALL,
	CRATE_FREEZE,
	CRATE_EMP,
	CRATE_DUCK,
	CRATE_BOOM,
	CRATE_POISON
};

typedef struct
{
	const char* m_pName;
	const char* m_pDesc;

	int m_Price;
	int m_MaxHealth;

	float m_Width;
	float m_Height;

	int m_MaxAimRange;
	int m_ReloadTime;
} t_BuildingsInfo;

extern t_BuildingsInfo aBuildingsInfo[];

#endif