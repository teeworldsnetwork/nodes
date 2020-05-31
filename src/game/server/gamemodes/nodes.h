/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_NODES_H
#define GAME_SERVER_GAMEMODES_NODES_H
#include <vector>
#include <game/buildings_info.h>
#include <game/server/gamecontroller.h>
#include <game/server/entities/building.h>

class CGameControllerNODES : public IGameController
{
public:
	CGameControllerNODES(class CGameContext *pGameServer);

	virtual void Tick();
	virtual void OnReset();
	virtual void Snap(int SnappingClient);
	virtual void OnPlayerConnect(CPlayer* pPlayer);
	virtual void DoTeamChange(CPlayer* pPlayer, int Team, bool DoChatMsg);

	virtual bool OnEntity(int Index, vec2 Pos);

	virtual int OnCharacterDeath(class CCharacter* pVictim, class CPlayer* pKiller, int Weapon);

	int KillsLeft(int Team);

	void PurgeBuildings();
	void DoNodesWincheck();
	void DoFallout(vec2 Pos);
	void FakeGameInfo(int ClientID);
	void EnterSpawnQueue(CPlayer* pPlayer);
	void LeaveSpawnQueue(CPlayer* pPlayer);
	void SetTechLevel(int Team, int Level);
	void SetBuildPoints(int Team, int Points);
	void DestroyBuilding(CBuilding* pBuilding, bool Init = false);

	bool BuildBuilding(vec2 Pos, int Type, int Team, int Owner, bool IgnorePower, bool Free);

	CPlayer* m_apSpawnQueue[NUM_TEAMS][MAX_CLIENTS];
	CBuilding* m_apBuildings[NUM_TEAMS][MAX_BUILDINGS];

	vec2 m_aFallout[32];
	vec2 m_aCrateSpawn[1024];

	bool m_Emp;
	bool m_Initiated;

	int64 m_EmpTick;

	int m_FalloutCount;
	int m_CrateSpawnCount;
	int m_aFalloutTimes[32];
	int m_InitBuildingsCount;
	int m_aSpawns[NUM_TEAMS];
	int m_aTechLevel[NUM_TEAMS];
	int m_aBuildPoints[NUM_TEAMS];
	int m_aBuildingsCount[NUM_TEAMS];
	int m_aSpawnQueueSize[NUM_TEAMS];

	struct {
		vec2 m_Pos;
		int m_Team;
		int m_Type;
	} m_InitBuildings[MAX_BUILDINGS];

	void ReinitBuildings();

	bool CanSpawnNodes(class CPlayer* pPlayer, int Team, vec2* pOutPos);
};
#endif
