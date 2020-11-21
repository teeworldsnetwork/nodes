#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/gamemodes/nodes.h>
#include <game/mapitems.h>
#include <game/buildings_info.h>

#include "character.h"
#include "crate.h"

static const char* BUF_NAMES[10] = { "", "Health", "Armor", "Construction", "Baseball", "Freeze", "EMP", "Duck", "Boom", "Poison" };

CCrate::CCrate(CGameWorld* pWorld, int Type, vec2 Pos)
	: CEntity(pWorld, CGameWorld::ENTTYPE_CRATE, Pos, ms_PhysSize)
{
	m_Type = Type;

	m_Alive = true;
	m_CreationTick = Server()->Tick();

	GameWorld()->InsertEntity(this);
}

void CCrate::Reset()
{
	Destroy();
}

void CCrate::Destroy()
{
	m_Alive = false;
	GameServer()->m_World.DestroyEntity(this);
}

bool CCrate::IsGrounded()
{
	if (GameServer()->Collision()->CheckPoint(m_Pos.x + GetProximityRadius() / 2, m_Pos.y + GetProximityRadius() / 2 + 5))
		return true;
	if (GameServer()->Collision()->CheckPoint(m_Pos.x - GetProximityRadius() / 2, m_Pos.y + GetProximityRadius() / 2 + 5))
		return true;
	return false;
}

CCharacter* CCrate::GetNearest(int MaxDist)
{
	CCharacter* CloseCharacters[MAX_CLIENTS];
	int Num = GameServer()->m_World.FindEntities(m_Pos, MaxDist, (CEntity**)CloseCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	int Disti = -1;
	float Dist = 10000000.0f;
	for (int i = 0; i < Num; i++)
	{
		if (!CloseCharacters[i])
			continue;

		if (GameServer()->Collision()->IntersectLine(m_Pos, CloseCharacters[i]->GetPos(), 0x0, 0x0))
			continue;

		float d = distance(CloseCharacters[i]->GetPos(), m_Pos);
		if (d < Dist)
		{
			Disti = i;
			Dist = d;
		}
	}

	if (Disti == -1)
		return 0;

	return CloseCharacters[Disti];
}

void CCrate::Tick()
{
	// handle death-tiles and leaving gamelayer
	if (GameServer()->Collision()->GetCollisionAt(m_Pos.x + GetProximityRadius() / 2.f, m_Pos.y - GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x + GetProximityRadius() / 2.f, m_Pos.y + GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x - GetProximityRadius() / 2.f, m_Pos.y - GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x - GetProximityRadius() / 2.f, m_Pos.y + GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameLayerClipped(m_Pos))
	{
		Die();
	}

	m_Grounded = IsGrounded();

	bool WasGrounded = m_Grounded;

	if (!m_Grounded)
	{
		int k = 3;
		while (!m_Grounded && k-- > 0)
		{
			m_Pos.y += 1;
			m_Grounded = IsGrounded();
		}
	}

	if (m_Grounded && !WasGrounded)
		GameServer()->CreateSound(m_Pos, SOUND_CRATE_IMPACT);

	WasGrounded = m_Grounded;

	if (!m_Alive)
		return;

	CCharacter* pChr = GetNearest(28);
	CGameControllerNODES* pNodes = ((CGameControllerNODES*)GameServer()->m_pController);

	if (pChr)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "%s", BUF_NAMES[m_Type]);
		GameServer()->SendBroadcast(aBuf, pChr->GetPlayer()->GetCID());

		switch (m_Type)
		{
		case CRATE_HEALTH:
		{
			pChr->IncreaseHealth(10);
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH);
		} break;

		case CRATE_ARMOR:
		{
			pChr->IncreaseArmor(10);
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
		} break;

		case CRATE_CONSTRUCTION:
		{
			pChr->m_CrateBuff = CRATE_CONSTRUCTION;
			pChr->m_CrateBuffTick = Server()->Tick();
			pChr->SetFrozen(false);
		} break;

		case CRATE_BASEBALL:
		{
			pChr->m_CrateBuff = CRATE_BASEBALL;
			pChr->m_CrateBuffTick = Server()->Tick();
			pChr->SetFrozen(false);
			pChr->SetActiveWeapon(WEAPON_HAMMER);
		} break;

		case CRATE_FREEZE:
		{
			pChr->m_CrateBuff = 0;
			pChr->SetFrozen(false);

			CCharacter* pCloseCharacters[MAX_CLIENTS];
			int Num = GameServer()->m_World.FindEntities(pChr->GetPos(), Config()->m_SvFreezeRadius, (CEntity**)pCloseCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for (int i = 0; i < Num; i++)
			{
				if (!pCloseCharacters[i])
					continue;

				if (pCloseCharacters[i]->GetPlayer()->GetTeam() != pChr->GetPlayer()->GetTeam())
				{
					pCloseCharacters[i]->SetFrozen(true);
					pCloseCharacters[i]->m_CrateBuff = CRATE_FREEZE;
					pCloseCharacters[i]->m_CrateBuffTick = Server()->Tick();
					pCloseCharacters[i]->m_CrateBuffMisc = (int)(frandom() * 2);
					GameServer()->CreateSound(pCloseCharacters[i]->GetPos(), SOUND_CRATE_ICE);
				}
			}

			GameServer()->CreateSound(m_Pos, SOUND_CRATE_ICE);
			GameServer()->CreateEffect(m_Pos, EFFECT_FREEZE);
		} break;

		case CRATE_EMP:
		{
			pNodes->m_Emp = true;
			pNodes->m_EmpTick = Server()->Tick();
			GameServer()->CreateEffect(m_Pos, EFFECT_RAMPAGE);
			GameServer()->CreateSoundGlobal(SOUND_CRATE_EMP, -1);
		} break;

		case CRATE_DUCK:
		{
			pChr->SetFrozen(true);
			pChr->m_CrateBuff = CRATE_FREEZE;
			pChr->m_CrateBuffTick = Server()->Tick();
			pChr->m_CrateBuffMisc = (int)(frandom() * 2);
			pChr->SetHookState(-1);
			pChr->SetHookedPlayer(-1);
			GameServer()->CreateSound(m_Pos, SOUND_CRATE_ICE);
		} break;

		case CRATE_BOOM:
		{
			GameServer()->CreateExplosion(m_Pos, -4, WEAPON_WORLD, 5);
			GameServer()->CreateExplosion(m_Pos, -4, WEAPON_WORLD, 5);
			GameServer()->CreateExplosion(m_Pos, -4, WEAPON_WORLD, 5);
			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		} break;

		case CRATE_POISON:
		{
			pChr->m_Ill = true;
			pChr->m_IllInfecter = -1;
		}
		break;
		}

		Die();
		return;
	}

	if (Server()->Tick() > m_CreationTick + 20 * Server()->TickSpeed())
	{
		GameServer()->CreateExplosion(m_Pos, -1, WEAPON_WORLD, 0);
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		Die();
	}
}

void CCrate::Die()
{
	Destroy();
}

void CCrate::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient, m_Pos))
		return;

	CNetObj_Crate* pCrate = static_cast<CNetObj_Crate*>(Server()->SnapNewItem(NETOBJTYPE_CRATE, GetID(), sizeof(CNetObj_Crate)));
	if (!pCrate)
		return;

	pCrate->m_X = (int)m_Pos.x;
	pCrate->m_Y = (int)m_Pos.y;
	pCrate->m_Type = m_Type;
	pCrate->m_Grounded = m_Grounded;
}