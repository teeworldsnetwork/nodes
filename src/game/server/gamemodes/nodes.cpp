/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/entities/character.h>
#include <game/server/entities/crate.h>
#include "nodes.h"

CGameControllerNODES::CGameControllerNODES(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "Nodes";
	m_GameFlags = GAMEFLAG_TEAMS;

	m_Emp = false;
	m_FalloutCount = 0;
	m_CrateSpawnCount = 0;
	m_InitBuildingsCount = 0;

	for (int t = 0; t < NUM_TEAMS; t++)
	{
		for (int i = 0; i < MAX_BUILDINGS; i++)
			m_apBuildings[t][i] = 0;

		m_aBuildingsCount[t] = 0;
		m_aBuildPoints[t] = 0;
		m_aTechLevel[t] = 0;
	}

	StartMatch();
}

void CGameControllerNODES::OnReset()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->m_RespawnDisabled = false;
			GameServer()->m_apPlayers[i]->Respawn();
			GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick() + Server()->TickSpeed() / 2;

			if (m_RoundCount == 0)
			{
				GameServer()->m_apPlayers[i]->m_Score = 0;
				GameServer()->m_apPlayers[i]->m_ScoreStartTick = Server()->Tick();
			}

			GameServer()->m_apPlayers[i]->m_IsReadyToPlay = true;
		}
	}

	dbg_msg("nodes", "Resetting...");
	ReinitBuildings();
}

void CGameControllerNODES::Tick()
{
	IGameController::Tick();

	DoNodesWincheck();
	PurgeBuildings();

	bool aReactors[NUM_TEAMS];
	for (int t = 0; t < NUM_TEAMS; t++)
		aReactors[t] = false;

	for (int t = 0; t < NUM_TEAMS; t++)
	{
		m_aSpawns[t] = 0;
		for (int i = 0; i < m_aBuildingsCount[t]; i++)
		{
			if (m_apBuildings[t][i]->Type() == B_REACTOR && m_apBuildings[t][i]->Alive())
				aReactors[t] = true;
			else if (m_apBuildings[t][i]->Type() == B_SPAWN && m_apBuildings[t][i]->Alive() && !m_apBuildings[t][i]->Deconstruction())
				m_aSpawns[t]++;
		}
	}

	if (Server()->Tick() % Server()->TickSpeed() == 0)
	{
		// handle spawns
		for (int t = 0; t < NUM_TEAMS; t++)
		{
			int Mod = (aReactors[t] ? (3 * Server()->TickSpeed()) : (6 * Server()->TickSpeed()));
			if (Server()->Tick() % Mod == 0)
			{
				if (m_aSpawnQueueSize[t] > 0)
				{
					CPlayer* ToSpawn = m_apSpawnQueue[t][0];
					if (!ToSpawn->GetCharacter())
						ToSpawn->TryRespawn();

					LeaveSpawnQueue(ToSpawn);
				}
			}

			for (int i = 0; i < m_aSpawnQueueSize[t]; i++)
			{
				int SpawnTime = i * (Mod / Server()->TickSpeed()) + (Mod - Server()->Tick() % Mod) / Server()->TickSpeed();
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Respawn time: %d", SpawnTime);
				GameServer()->SendBroadcast(aBuf, m_apSpawnQueue[t][i]->GetCID());
			}
		}

		if (Server()->Tick() % 75 == 0)
		{
			for (int i = 0; i < m_FalloutCount; i++)
			{
				if (m_aFalloutTimes[i] + 20 * Server()->TickSpeed() < Server()->Tick())
				{
					m_aFallout[i] = m_aFallout[m_FalloutCount - 1];
					m_aFalloutTimes[i] = m_aFalloutTimes[--m_FalloutCount];
				}
			}

			for (int i = 0; i < m_FalloutCount; i++)
			{
				// fallout building damage
				CBuilding* pBuildings[256];
				int Num = GameServer()->m_World.FindEntities(m_aFallout[i], 450.0f, (CEntity**)pBuildings, 256, CGameWorld::ENTTYPE_BUILDING);
				for (int j = 0; j < Num; j++)
				{
					float Dist = distance(pBuildings[j]->GetPos(), m_aFallout[i]);
					int Dmg = (int)((-5.0f / 450.0f) * Dist + 5.0f);
					pBuildings[j]->TakeDamage(vec2(0, 0), Dmg, -5, -1, WEAPON_WORLD);
				}

				// fallout character damage
				CCharacter* apCharacters[MAX_CLIENTS];
				Num = GameServer()->m_World.FindEntities(m_aFallout[i], 450.0f, (CEntity**)apCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
				for (int j = 0; j < Num; j++)
				{
					float Dist = distance(apCharacters[j]->GetPos(), m_aFallout[i]);
					int Dmg = (int)((-5.0f / 450.0f) * Dist + 5.0f);
					apCharacters[j]->TakeDamage(vec2(0, 0), vec2(0, 0), Dmg, -5, WEAPON_WORLD, -1);
					if (!apCharacters[j]->m_Ill)
					{
						apCharacters[j]->m_Ill = true;
						apCharacters[j]->m_IllInfecter = -1;
					}
				}
			}
		}
	}

	if (m_CrateSpawnCount > 0 && frandom() < Config()->m_SvCrateProbability / 1000.0f / 50.0f)
	{
		vec2 CratePos = m_aCrateSpawn[rand() % m_CrateSpawnCount];
		int Crate = (int)(frandom() * 9) + 1;
		new CCrate(&GameServer()->m_World, Crate, CratePos);
		GameServer()->CreateSound(CratePos, SOUND_CRATE_SPAWN);
	}

	if (m_Emp && (Server()->Tick() > m_EmpTick + Config()->m_SvEmpDuration * Server()->TickSpeed()))
		m_Emp = false;
}

void CGameControllerNODES::ReinitBuildings()
{
	dbg_msg("nodes", "Initializing buildings on startup");

	for (int t = 0; t < NUM_TEAMS; t++)
	{
		while (m_aBuildingsCount[t] > 0)
			DestroyBuilding(m_apBuildings[t][0], true);

		m_aBuildingsCount[t] = 0;
		SetTechLevel(t, 1);
		SetBuildPoints(t, Config()->m_SvStartBuildpoints);
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i]) {
			GameServer()->m_apPlayers[i]->m_Rage = 0;
		}
	}

	for (int i = 0; i < m_InitBuildingsCount; i++)
		BuildBuilding(m_InitBuildings[i].m_Pos, m_InitBuildings[i].m_Type, m_InitBuildings[i].m_Team, -1, true, true);

	for (int t = 0; t < NUM_TEAMS; t++)
		m_aSpawnQueueSize[t] = 0;

	m_Emp = false;
	m_EmpTick = 0;
	m_FalloutCount = 0;
	m_Initiated = true;
}

void CGameControllerNODES::DestroyBuilding(CBuilding* pBuilding, bool OnInit)
{
	for (int t = 0; t < NUM_TEAMS; t++)
	{
		m_aSpawns[t] = 0;

		for (int i = 0; i < m_aBuildingsCount[t]; i++)
		{
			if (m_apBuildings[t][i] == pBuilding)
			{
				if (!pBuilding->WasFree())
					m_aBuildPoints[t] += aBuildingsInfo[m_apBuildings[t][i]->Type()].m_Price;

				delete m_apBuildings[t][i];

				m_apBuildings[t][i] = m_apBuildings[t][m_aBuildingsCount[t] - 1];
				m_aBuildingsCount[t]--;
			}
		}

		for (int i = 0; i < m_aBuildingsCount[t]; i++)
		{
			if (m_apBuildings[t][i]->Type() == B_SPAWN && m_apBuildings[t][i]->Alive())
				m_aSpawns[t]++;
		}

		if (m_aSpawns[t] == 0 && !OnInit)
			GameServer()->SendChat(-1, CHAT_TEAM, t, "There are no working spawns left!");
	}
}

bool CGameControllerNODES::OnEntity(int Index, vec2 Pos)
{
	if (IGameController::OnEntity(Index, Pos))
		return true;

	int NODE_BASE = 17; // entities, first nodes entity = 17
	int Type = -1;
	int Team = 0;

	if (Index >= NODE_BASE && Index < NODE_BASE + 12)
	{
		Type = Index - NODE_BASE;
		Team = TEAM_RED;
	}
	else if (Index >= NODE_BASE + 16 && Index < NODE_BASE + 28)
	{
		Type = Index - NODE_BASE - 16;
		Team = TEAM_BLUE;
	}
	else if (Index == NODE_BASE + 32)
		m_aCrateSpawn[m_CrateSpawnCount++] = Pos;

	if (Type != -1 && m_InitBuildingsCount < MAX_BUILDINGS)
	{
		m_InitBuildings[m_InitBuildingsCount].m_Pos = Pos;
		m_InitBuildings[m_InitBuildingsCount].m_Type = Type;
		m_InitBuildings[m_InitBuildingsCount].m_Team = Team;
		m_InitBuildingsCount++;
	}

	return true;
}

bool CGameControllerNODES::BuildBuilding(vec2 Pos, int Type, int Team, int Owner, bool IgnorePower, bool Free)
{
	if (Team == TEAM_SPECTATORS || m_aBuildingsCount[Team] > MAX_BUILDINGS)
		return false;

	if (m_SuddenDeath)
	{
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "You cannot build anything during sudden death");
		return false;
	}

	int Space = 0;
	vec2 TempPos = Pos;
	while (!GameServer()->Collision()->CheckPoint(TempPos) && Space < 32)
	{
		TempPos.y -= 32;
		Space++;
	}

	if (aBuildingsInfo[Type].m_Height > Space * 32)
	{
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "There's not enough space to build here");
		return false;
	}

	int Techlevel = Type / 4 + 1;
	if (Techlevel > m_aTechLevel[Team] && !IgnorePower)
	{
		dbg_msg("nodes", "Techlevel too low! ClientID: %d, Team: %d, Building: %s, Techlevel: %d/%d", Owner, Team, aBuildingsInfo[Type].m_pName, m_aTechLevel[Team], Techlevel);
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "Your team need need a higher techlevel to build this");
		return false;
	}

	if (aBuildingsInfo[Type].m_Price > m_aBuildPoints[Team])
	{
		dbg_msg("nodes", "Buildpoints too low! ClientID: %d, Team: %d, Building: %s, Buildpoints: %d/%d", Owner, Team, aBuildingsInfo[Type].m_pName, m_aBuildPoints[Team], aBuildingsInfo[Type].m_Price);
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "Your team has insufficient build points");
		return false;
	}

	bool Reactor = false;
	bool Powered = false;
	int Teleports = 0;
	for (int t = 0; t < NUM_TEAMS; t++)
	{
		for (int i = 0; i < m_aBuildingsCount[t]; i++)
		{
			if (distance(m_apBuildings[t][i]->GetPos(), Pos) < m_apBuildings[t][i]->ms_PhysSize * 2)
			{
				dbg_msg("nodes", "Blocked. ClientID: %d, Team: %d, Building: %s, Distance: %.2f, Needed: %.2f", Owner, Team, aBuildingsInfo[Type].m_pName, distance(m_apBuildings[t][i]->GetPos(), Pos), m_apBuildings[t][i]->ms_PhysSize * 2.0f);
				if (GameServer()->m_apPlayers[Owner])
					GameServer()->SendChatMessage(Owner, "This spot is blocked");
				return false;
			}

			if (Team != t && distance(m_apBuildings[t][i]->GetPos(), Pos) < 400.0f)
			{
				dbg_msg("nodes", "Too close to enemy. ClientID: %d, Team: %d, Building: %s", Owner, Team, aBuildingsInfo[Type].m_pName);
				if (GameServer()->m_apPlayers[Owner])
					GameServer()->SendChatMessage(Owner, "This spot is too close to the enemy!");
				return false;
			}

			if (Team == t && m_apBuildings[t][i]->Type() == B_REACTOR)
				Reactor = true;

			if (Team == t && ((m_apBuildings[t][i]->Type() == B_REACTOR && distance(Pos, m_apBuildings[t][i]->GetPos()) < 750.0f) || (m_apBuildings[t][i]->Type() == B_REPEATER && distance(Pos, m_apBuildings[t][i]->GetPos()) < 500.0f)) && m_apBuildings[t][i]->Alive())
				Powered = true;

			if (Team == t && m_apBuildings[t][i]->Type() == B_TELEPORT)
				Teleports++;
		}
	}

	if (!Reactor && Type != B_REACTOR && !IgnorePower)
	{
		dbg_msg("nodes", "No reactor. ClientID: %d, Team: %d, Building: %s", Owner, Team, aBuildingsInfo[Type].m_pName);
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "You need to build a reactor first");
		return false;
	}

	if (Reactor && Type == B_REACTOR && !IgnorePower)
	{
		dbg_msg("nodes", "Second reactor. ClientID: %d, Team: %d, Building: %s", Owner, Team, aBuildingsInfo[Type].m_pName);
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "Your team can only have one reactor");
		return false;
	}

	if (!Powered && Type != B_REACTOR && Type != B_REPEATER && !IgnorePower)
	{
		dbg_msg("nodes", "No power. ClientID: %d, Team: %d, Building: %s", Owner, Team, aBuildingsInfo[Type].m_pName);
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "This spot is not powered. Build a repeater or reactor first");
		return false;
	}

	if (Type == B_TELEPORT && Teleports >= 2)
	{
		dbg_msg("nodes", "Too much teleports. ClientID: %d, Team: %d, Building: %s", Owner, Team, aBuildingsInfo[Type].m_pName);
		if (GameServer()->m_apPlayers[Owner])
			GameServer()->SendChatMessage(Owner, "Your team can only have two teleporters");
		return false;
	}

	int BuildingOwner = Owner >= 0 ? Owner : -1;
	int BuildingHealth = IgnorePower ? aBuildingsInfo[Type].m_MaxHealth : 1;
	bool BuildingAlive = IgnorePower ? true : false;

	CBuilding* pBuilding = new CBuilding(&GameServer()->m_World, Type, Pos, Team, BuildingOwner, BuildingHealth, BuildingAlive, Free);
	m_apBuildings[Team][m_aBuildingsCount[Team]++] = pBuilding;

	if (!Free)
		m_aBuildPoints[Team] -= aBuildingsInfo[Type].m_Price;

	dbg_msg("nodes", "New building. ClientID: %d, Team: %d, Building: %s", Owner, Team, aBuildingsInfo[Type].m_pName);
	return true;
}

void CGameControllerNODES::EnterSpawnQueue(CPlayer* pPlayer)
{
	if (pPlayer->GetTeam() != TEAM_RED && pPlayer->GetTeam() != TEAM_BLUE)
		return;

	for (int i = 0; i < m_aSpawnQueueSize[pPlayer->GetTeam()]; i++)
	{
		if (m_apSpawnQueue[pPlayer->GetTeam()][i] == pPlayer)
			return;
	}

	m_apSpawnQueue[pPlayer->GetTeam()][m_aSpawnQueueSize[pPlayer->GetTeam()]++] = pPlayer;
}

void CGameControllerNODES::LeaveSpawnQueue(CPlayer* pPlayer)
{
	for (int t = 0; t < 2; t++)
	{
		for (int i = m_aSpawnQueueSize[t] - 1; i >= 0; i--)
		{
			if (m_apSpawnQueue[t][i] == pPlayer)
			{
				for (int j = i; j < m_aSpawnQueueSize[t] - 1; j++)
					m_apSpawnQueue[t][j] = m_apSpawnQueue[t][j + 1];

				m_aSpawnQueueSize[t]--;
			}
		}
	}
}

bool CGameControllerNODES::CanSpawnNodes(class CPlayer* pPlayer, int Team, vec2* pOutPos)
{
	// spectators can't spawn
	if (Team == TEAM_SPECTATORS || GameServer()->m_World.m_Paused || GameServer()->m_World.m_ResetRequested)
		return false;

	CSpawnEval Eval;

	// all we need to do is to find the closest node and find a free spawn.
	Eval.m_FriendlyTeam = Team;

	int BestSpawn = -1;
	if (pPlayer->m_SelectSpawn)
	{
		if (pPlayer->m_WantedSpawn < m_aBuildingsCount[Team] && m_apBuildings[Team][pPlayer->m_WantedSpawn]->Type() == B_SPAWN && m_apBuildings[Team][pPlayer->m_WantedSpawn]->Alive() && m_apBuildings[Team][pPlayer->m_WantedSpawn]->Power())
			BestSpawn = pPlayer->m_WantedSpawn;
	}
	else
	{
		// find spawn with lowest costs
		float BestScore = 100000000.0f;

		for (int i = 0; i < m_aBuildingsCount[Team]; i++)
		{
			if (m_apBuildings[Team][i]->Type() != B_SPAWN || !m_apBuildings[Team][i]->Alive() || !m_apBuildings[Team][i]->Power())
				continue;

			float Score = distance(pPlayer->m_ViewPos, m_apBuildings[Team][i]->GetPos());
			if (Score < BestScore)
			{
				BestScore = Score;
				BestSpawn = i;
			}
		}
	}

	if (BestSpawn == -1)
		return false;

	*pOutPos = vec2(m_apBuildings[Team][BestSpawn]->GetPos().x, m_apBuildings[Team][BestSpawn]->GetPos().y - 80);
	m_apBuildings[Team][BestSpawn]->SetSpawnAnim(3);
	m_apBuildings[Team][BestSpawn]->SetSpawnAnimDecay(50);
	return true;
}

void CGameControllerNODES::SetTechLevel(int Team, int Level)
{
	m_aTechLevel[Team] = Level;
	GameServer()->Collision()->SetTechLevel(Team, Level);

	CNetMsg_Sv_TechlevelUpdate Msg;
	Msg.m_TechRed = m_aTechLevel[TEAM_RED];
	Msg.m_TechBlue = m_aTechLevel[TEAM_BLUE];
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);

	dbg_msg("nodes", "updated techlevels %d -> %d", Team, Level);
}

void CGameControllerNODES::SetBuildPoints(int Team, int Points)
{
	m_aBuildPoints[Team] = Points;

	dbg_msg("nodes", "updated buildpoints %d -> %d", Team, Points);
}

void CGameControllerNODES::DoFallout(vec2 Pos)
{
	GameServer()->CreateEffect(Pos, EFFECT_FALLOUT);
	m_aFallout[m_FalloutCount++] = Pos;
	m_aFalloutTimes[m_FalloutCount - 1] = Server()->Tick();
}

void CGameControllerNODES::FakeGameInfo(int ClientID)
{
	if (!GameServer()->m_apPlayers[ClientID])
		return;

	int Team = GameServer()->m_apPlayers[ClientID]->GetTeam();

	if (Team != TEAM_RED && Team != TEAM_BLUE)
		return;

	CNetMsg_Sv_GameInfo GameInfoMsg;
	GameInfoMsg.m_GameFlags = m_GameFlags;

	if (m_aTechLevel[Team] < 3)
		GameInfoMsg.m_ScoreLimit = KillsLeft(Team) + 1;
	else
		GameInfoMsg.m_ScoreLimit = 0;

	GameInfoMsg.m_TimeLimit = m_GameInfo.m_TimeLimit;
	GameInfoMsg.m_MatchNum = m_GameInfo.m_MatchNum;
	GameInfoMsg.m_MatchCurrent = m_GameInfo.m_MatchCurrent;

	Server()->SendPackMsg(&GameInfoMsg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ClientID);
}

int CGameControllerNODES::KillsLeft(int Team)
{
	if (Team != TEAM_RED && Team != TEAM_BLUE)
		return 0;

	if (m_aTechLevel[Team] >= 3)
		return 0;

	int pl[2] = { 0, 0 };
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!GameServer()->m_apPlayers[i])
			continue;

		int plTeam = GameServer()->m_apPlayers[i]->GetTeam();

		if (plTeam != TEAM_RED && plTeam != TEAM_BLUE) // that's a crashbug in 0.5 btw
			continue;

		pl[plTeam]++;
	}

	if (pl[Team] < 0)
		return 0;

	return 3 * pl[Team] * (m_aTechLevel[Team] * m_aTechLevel[Team]) - m_aTeamscore[Team];
}

int CGameControllerNODES::OnCharacterDeath(class CCharacter* pVictim, class CPlayer* pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	if (pKiller && Weapon != WEAPON_GAME)
	{
		if (pKiller != pVictim->GetPlayer() && pKiller->GetTeam() != pVictim->GetPlayer()->GetTeam())
			m_aTeamscore[pKiller->GetTeam() & 1]++;
	}

	for (int i = 0; i < NUM_TEAMS; i++)
	{
		while (KillsLeft(i) < 0 && m_aTechLevel[i] < 3)
		{
			SetTechLevel(i, m_aTechLevel[i] + 1);

			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Your team reached a new techlevel: %d", m_aTechLevel[i]);
			GameServer()->SendChat(-1, CHAT_TEAM, i, aBuf);

			GameServer()->DoTeamSound(SOUND_CTF_CAPTURE, i);
		}
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
		FakeGameInfo(i);

	return 0;
}

void CGameControllerNODES::OnPlayerConnect(CPlayer* pPlayer)
{
	IGameController::OnPlayerConnect(pPlayer);

	if (!pPlayer)
		return;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!GameServer()->m_apPlayers[i])
			continue;

		if (pPlayer->GetTeam() != GameServer()->m_apPlayers[i]->GetTeam())
			continue;

		FakeGameInfo(i);
	}
}

void CGameControllerNODES::DoTeamChange(CPlayer* pPlayer, int Team, bool DoChatMsg)
{
	IGameController::DoTeamChange(pPlayer, Team, DoChatMsg);

	pPlayer->m_SelectSpawn = false;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!GameServer()->m_apPlayers[i])
			continue;

		if (Team != GameServer()->m_apPlayers[i]->GetTeam())
			continue;

		FakeGameInfo(i);
	}
}

void CGameControllerNODES::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameDataTeam* pGameDataTeam = static_cast<CNetObj_GameDataTeam*>(Server()->SnapNewItem(NETOBJTYPE_GAMEDATATEAM, 0, sizeof(CNetObj_GameDataTeam)));
	if (!pGameDataTeam)
		return;

	if (GameServer()->m_apPlayers[SnappingClient])
	{
		int Team = GameServer()->m_apPlayers[SnappingClient]->GetTeam();

		pGameDataTeam->m_TeamscoreRed = m_aBuildPoints[Team];
		pGameDataTeam->m_TeamscoreBlue = m_aTechLevel[Team];
	}
	else
	{
		pGameDataTeam->m_TeamscoreRed = 0;
		pGameDataTeam->m_TeamscoreBlue = 0;
	}
}

void CGameControllerNODES::PurgeBuildings()
{
	for (int t = 0; t < NUM_TEAMS; t++)
	{
		for (int i = 0; i < m_aBuildingsCount[t]; i++)
		{
			// player leaves the server
			// his buildings are now owned by the server
			// so everybody can deconstruct them
			// reactors are also available for everyone
			if ((m_apBuildings[t][i]->Owner() >= 0 && m_apBuildings[t][i]->Owner() < MAX_CLIENTS && !GameServer()->m_apPlayers[m_apBuildings[t][i]->Owner()])
				|| m_apBuildings[t][i]->Type() == B_REACTOR)
				m_apBuildings[t][i]->SetOwner(-1);
		}
	}
}

void CGameControllerNODES::DoNodesWincheck()
{
	if (IsGameRunning() && m_Initiated)
	{
		int aSpawns[NUM_TEAMS];
		int aLiving[NUM_TEAMS];

		for (int t = 0; t < NUM_TEAMS; t++)
		{
			aSpawns[t] = 0;
			aLiving[t] = 0;
		}

		for (int t = 0; t < NUM_TEAMS; t++)
		{
			for (int i = 0; i < m_aBuildingsCount[t]; i++)
			{
				if (m_apBuildings[t][i]->Type() == B_SPAWN && m_apBuildings[t][i]->Alive())
					aSpawns[t]++;
			}
		}

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (!GameServer()->m_apPlayers[i])
				continue;

			if (GameServer()->m_apPlayers[i]->GetTeam() != TEAM_RED && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_BLUE)
				continue;

			if (!GameServer()->GetPlayerChar(i))
				continue;

			if (!GameServer()->GetPlayerChar(i)->IsAlive())
				continue;

			aLiving[GameServer()->m_apPlayers[i]->GetTeam()]++;
		}

		for (int t = 0; t < NUM_TEAMS; t++)
		{
			if (aSpawns[t] == 0 && aLiving[t] == 0)
			{
				m_aTeamscore[t] = 0;
				m_aTeamscore[t ^ 1] = 1;
				m_Initiated = false;
				EndRound();
			}
		}
	}
}