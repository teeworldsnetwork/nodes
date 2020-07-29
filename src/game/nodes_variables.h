/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_NODES_VARIABLES_H
#define GAME_NODES_VARIABLES_H
#undef GAME_NODES_VARIABLES_H // this file will be included several times

MACRO_CONFIG_INT(SvStartBuildpoints, sv_start_buildpoints, 1000, 0, 1000000, CFGFLAG_SAVE | CFGFLAG_SERVER, "Buildpoints on game start")
MACRO_CONFIG_INT(SvBuildDelay, sv_build_delay, 10, 0, 1000000, CFGFLAG_SAVE | CFGFLAG_SERVER, "Time delay before being able to build again")
MACRO_CONFIG_INT(SvCrateProbability, sv_crate_probability, 50, 0, 1000, CFGFLAG_SAVE | CFGFLAG_SERVER, "Probability per second in permil that a crate is created")
MACRO_CONFIG_INT(SvFreezeRadius, sv_freeze_radius, 200, 0, 100000, CFGFLAG_SAVE | CFGFLAG_SERVER, "Radius of the freeze effect")
MACRO_CONFIG_INT(SvEmpDuration, sv_emp_duration, 5, 0, 1000000, CFGFLAG_SAVE | CFGFLAG_SERVER, "Duration of the EMP effect")
MACRO_CONFIG_INT(SvRageModifier, sv_rage_modifier, 1, 0, 100, CFGFLAG_SAVE | CFGFLAG_SERVER, "Rage points per hit")
MACRO_CONFIG_INT(SvVeteranTurrets, sv_veteran_turrets, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_SERVER, "Veteran mode for turrets")
MACRO_CONFIG_INT(SvScoreTurrets, sv_score_turrets, 1, 0, 1, CFGFLAG_SAVE | CFGFLAG_SERVER, "Players score with turrets")
MACRO_CONFIG_INT(SvOwnerProtection, sv_owner_protection, 1, 0, 1, CFGFLAG_SAVE | CFGFLAG_SERVER, "Team-Buildings self-destruct limited to their owners")
MACRO_CONFIG_INT(SvDeconstructTime, sv_deconstruct_time, 10, 0, 900, CFGFLAG_SAVE | CFGFLAG_SERVER, "Time delay before being able to deconstruct a building again")
MACRO_CONFIG_INT(SvBuildingsFriendlyFire, sv_buildings_friendly_fire, 1, 0, 1, CFGFLAG_SAVE | CFGFLAG_SERVER, "Friendly fire on buildings")
MACRO_CONFIG_INT(SvBuildSpeed, sv_buildspeed, 2, 0, 10, CFGFLAG_SAVE | CFGFLAG_SERVER, "Building speed")

#endif