#include <generated/server_data.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/gamemodes/nodes.h>
#include <game/server/entities/character.h>
#include <game/buildings_info.h>
#include "projectile.h"
#include "building.h"

CBuilding::CBuilding(CGameWorld* pWorld, int Type, vec2 Pos, int Team, int Owner, int Health, bool Alive, bool Free)
	: CEntity(pWorld, CGameWorld::ENTTYPE_BUILDING, Pos, ms_PhysSize)
{
	m_Type = Type;
	m_Team = Team;
	m_Owner = Owner;
	m_Health = Health;
	m_Alive = Alive;
	m_Free = Free;

	m_Angle = 0;
	m_VeteranTTLBonus = 1.0f;

	m_MaxHealth = aBuildingsInfo[m_Type].m_MaxHealth;
	m_MaxAimRange = aBuildingsInfo[m_Type].m_MaxAimRange;
	m_ReloadTime = aBuildingsInfo[m_Type].m_ReloadTime;

	m_Width = aBuildingsInfo[m_Type].m_Width;
	m_Height = aBuildingsInfo[m_Type].m_Height;

	GameWorld()->InsertEntity(this);
}

void CBuilding::Reset()
{
	Destroy();
}

void CBuilding::Destroy()
{
	m_Alive = false;
}

void CBuilding::Tick()
{
	// handle death-tiles and leaving gamelayer
	if (GameServer()->Collision()->GetCollisionAt(m_Pos.x + GetProximityRadius() / 2.f, m_Pos.y - GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x + GetProximityRadius() / 2.f, m_Pos.y + GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x - GetProximityRadius() / 2.f, m_Pos.y - GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x - GetProximityRadius() / 2.f, m_Pos.y + GetProximityRadius() / 2.f) & CCollision::COLFLAG_DEATH ||
		GameLayerClipped(m_Pos))
	{
		Die(-1, WEAPON_WORLD);
	}

	if (!IsGrounded())
		Die(-1, WEAPON_WORLD);

	if (Server()->Tick() % 5 == 0 && m_Deconstruction)
	{
		if (((CGameControllerNODES*)GameServer()->m_pController)->m_aSpawns[m_Team] <= 0)
			m_Deconstruction = false;
		else if (!TakeDamage(vec2(0, 0), 1, -1, -1, WEAPON_SELF))
			return;
	}

	if (m_Decay > 0)
		m_Decay--;

	if (m_AnimDecay > 0)
		m_AnimDecay--;

	if (m_SpawnAnimDecay > 0)
		m_SpawnAnimDecay--;

	HandlePower();

	if (!m_Power && (m_Type == B_REPEATER || m_Type == B_TELEPORT))
		m_Anim = 0;

	if (!m_Power)
		m_Hit = 0;

	if (!m_Alive || (!m_Power && m_Type != B_SHIELD))
		return;

	switch (m_Type)
	{
	case B_REACTOR:
	{
		if (m_AnimDecay == 0)
		{
			m_Anim++;
			if (m_Anim > 3)
			{
				m_Anim = 0;
				m_AnimDecay = 100;
			}
			else
				m_AnimDecay = 10;
		}
	} break;

	case B_REPEATER:
	{
		if (!m_Power)
			m_Anim = 0;
		else
		{
			if (m_AnimDecay == 0)
			{
				m_Anim++;
				if (m_Anim > 1)
				{
					m_Anim = 0;
					m_AnimDecay = 100;
				}
				else
					m_AnimDecay = 10;
			}
		}
	} break;

	case B_SPAWN:
	{
		m_Anim = m_Anim & 0xF;
		if (m_AnimDecay == 0)
		{
			m_Anim++;
			if (m_Anim > 6)
				m_Anim = 0;
			m_AnimDecay = 35;
		}

		if (m_SpawnAnim > 0 && m_SpawnAnimDecay == 0)
		{
			m_SpawnAnim--;
			m_SpawnAnimDecay = 4;
		}

		m_Anim = (m_SpawnAnim << 4) | m_Anim;
	} break;

	case B_AMMO_SHOTGUN:
	case B_AMMO_GRENADE:
	case B_AMMO_LASER:
	{
		CCharacter* pChr = GetNearest(ms_PhysSize, m_Team);
		if (pChr)
		{
			if (m_Anim < 3 && m_AnimDecay == 0)
			{
				m_Anim++;
				m_AnimDecay = 4;
			}

			if (m_Decay == 0 && m_Anim == 3)
			{
				m_Decay = 50;
				int Weapon = -1;

				if (m_Type == B_AMMO_SHOTGUN)
					Weapon = WEAPON_SHOTGUN;
				else if (m_Type == B_AMMO_GRENADE)
					Weapon = WEAPON_GRENADE;
				else if (m_Type == B_AMMO_LASER)
					Weapon = WEAPON_LASER;

				bool GotThisBefore = pChr->GotWeapon(Weapon);
				pChr->GiveWeapon(Weapon, 10);

				if (!GotThisBefore)
				{
					pChr->SetActiveWeapon(Weapon);
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
				}
				else
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE);
			}
		}
		else
		{
			if (m_Anim > 0 && m_AnimDecay == 0)
			{
				m_Anim--;
				m_AnimDecay = 4;
			}
		}
	} break;

	case B_TURRET_GUN:
	{
		vec2 RealPos = vec2(m_Pos.x, m_Pos.y - 74);
		vec2 TempPos = m_Pos;
		m_Pos = RealPos;

		CCharacter* pChr = GetNearest(m_MaxAimRange, m_Team ^ 1);
		m_Pos = TempPos;

		if (pChr)
		{
			m_Angle = angle(normalize(pChr->GetPos() - RealPos));
			if (m_Decay == 0)
			{
				m_VeteranShots++;
				if (m_VeteranShots == 100 && Config()->m_SvVeteranTurrets)
				{
					m_ReloadTime /= 2;
					m_MaxAimRange += 100;
				}
				else if (m_VeteranShots == 200 && Config()->m_SvVeteranTurrets)
					m_MaxAimRange += 25;

				int BulletOwner;
				if (Config()->m_SvScoreTurrets && m_Owner >= 0 && GameServer()->m_apPlayers[m_Owner] && GameServer()->m_apPlayers[m_Owner]->GetTeam() == m_Team)
					BulletOwner = m_Owner;
				else
				{
					m_Owner = -1; // player left or changed the team (no chance to own it anymore)
					BulletOwner = -1;
				}

				m_Decay = m_ReloadTime;
				float a = m_Angle;
				a += frandom() * 0.2f - 0.1f;

				new CProjectile(GameWorld(), WEAPON_GUN,
					PLAYER_TURRET_GUN, RealPos,
					vec2(cosf(a), sinf(a)),
					(int)(Server()->TickSpeed() * GameServer()->Tuning()->m_GunLifetime),
					1, false, 0, -1, WEAPON_GUN, false, BulletOwner);
				GameServer()->CreateSound(RealPos, SOUND_GUN_FIRE);
			}
		}
	} break;

	case B_TURRET_SHOTGUN:
	{
		vec2 RealPos = vec2(m_Pos.x, m_Pos.y - 66);
		vec2 TempPos = m_Pos;
		m_Pos = RealPos;

		CCharacter* pChr = GetNearest(m_MaxAimRange, m_Team ^ 1);
		m_Pos = TempPos;

		if (pChr)
		{
			m_Angle = angle(normalize(pChr->GetPos() - RealPos));
			if (m_Decay == 0)
			{
				m_VeteranShots++;
				if (m_VeteranShots == 100 && Config()->m_SvVeteranTurrets)
				{
					m_ReloadTime = m_ReloadTime - ((m_ReloadTime / 10) * 2);
					m_MaxAimRange += 100;
					m_VeteranTTLBonus = 1.8f;
				}
				else if (m_VeteranShots == 200 && Config()->m_SvVeteranTurrets)
				{
					m_ReloadTime = m_ReloadTime - ((m_ReloadTime / 10) * 2);
					m_MaxAimRange += 25;
				}

				m_Decay = m_ReloadTime;

				int ShotSpread = 2;
				int BulletOwner;
				if (Config()->m_SvScoreTurrets && m_Owner >= 0 && GameServer()->m_apPlayers[m_Owner] && GameServer()->m_apPlayers[m_Owner]->GetTeam() == m_Team)
					BulletOwner = m_Owner;
				else
				{
					m_Owner = -1; // player left or changed the team (no chance to own it anymore)
					BulletOwner = -1;
				}

				for (int i = -ShotSpread; i <= ShotSpread; i++)
				{
					float Spreading[] = { -0.185f, -0.070f, 0, 0.070f, 0.185f };
					float a = m_Angle;
					a += Spreading[i + 2];
					float v = 1 - (abs(i) / (float)ShotSpread);
					float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);

					new CProjectile(GameWorld(), WEAPON_SHOTGUN,
						PLAYER_TURRET_SHOTGUN, RealPos,
						vec2(cosf(a), sinf(a)) * Speed,
						(int)(Server()->TickSpeed() * GameServer()->Tuning()->m_ShotgunLifetime * m_VeteranTTLBonus),
						1, false, 0, -1, WEAPON_SHOTGUN, false, BulletOwner);
				}
				GameServer()->CreateSound(RealPos, SOUND_SHOTGUN_FIRE);
			}
		}
	} break;

	case B_HEALTH:
	case B_ARMOR:
	{
		CCharacter* pChr = GetNearest(ms_PhysSize, m_Team);
		if (pChr)
		{
			if (m_Decay == 0)
			{
				if (m_AnimDecay == 0 && m_Decay == 0 && m_Anim < 4)
				{
					m_AnimDecay = 3;
					m_Anim++;
				}
				else if (m_AnimDecay == 0)
				{
					if (m_Type == B_HEALTH)
						pChr->m_Ill = false;

					if (m_Type == B_HEALTH && pChr->Health() < 10)
					{
						m_Decay = 150;
						pChr->IncreaseHealth(3);
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH);
					}
					else if (m_Type == B_ARMOR && pChr->Armor() < 10)
					{
						m_Decay = 150;
						pChr->IncreaseArmor(3);
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
					}
				}
			}
		}
		else
		{
			if (m_AnimDecay == 0 && m_Decay == 0 && m_Anim > 0)
			{
				m_AnimDecay = 3;
				m_Anim--;
			}
		}
	} break;

	case B_SHIELD:
	{
		if (Server()->Tick() % Server()->TickSpeed() == 0 && m_Power)
		{
			if (m_Armor < 30)
				m_Armor++;
		}

		if (Server()->Tick() % 5 == 0)
		{
			if (m_Hit > 0)
				m_Hit--;

			if (m_Power && m_Armor > 0)
			{
				CProjectile* pProj[128];
				int Num = GameServer()->m_World.FindEntities(m_Pos, 250.0f, (CEntity**)pProj, 128, CGameWorld::ENTTYPE_PROJECTILE);
				for (int i = 0; i < Num && m_Armor > 0; i++)
				{
					if (pProj[i]->m_Bounce)
						continue;

					if ((pProj[i]->Owner() >= 0) && GameServer()->m_apPlayers[pProj[i]->Owner()] && GameServer()->m_apPlayers[pProj[i]->Owner()]->GetTeam() == m_Team)
						continue;

					if (pProj[i]->Owner() < 0)
						continue;

					pProj[i]->m_InitPos = pProj[i]->GetRealPos();
					pProj[i]->m_StartTick = Server()->Tick();
					pProj[i]->m_Direction = normalize(pProj[i]->GetRealPos() - m_Pos);
					pProj[i]->m_Bounce = 1;
					m_Hit = 3;

					m_Armor = clamp(m_Armor - (pProj[i]->m_Weapon == WEAPON_GRENADE ? 4 : 2), 0, 30);
					if (m_Armor == 0)
						m_Armor = -4;
				}
			}
		}

		m_Anim = (m_Armor > 0 ? (int)(9.0f * m_Armor / 30.0f) : 0) | (m_Hit << 4);
	} break;

	case B_TELEPORT:
	{
		if (m_Decay == 0)
		{
			CBuilding* pOther = 0;
			for (int i = 0; i < ((CGameControllerNODES*)GameServer()->m_pController)->m_aBuildingsCount[m_Team]; i++)
			{
				CBuilding* pBuilding = ((CGameControllerNODES*)GameServer()->m_pController)->m_apBuildings[m_Team][i];
				if (pBuilding != this && pBuilding->m_Type == B_TELEPORT && pBuilding->m_Alive && pBuilding->m_Power)
					pOther = pBuilding;
			}

			if (pOther)
			{
				bool IsEnemy = false;
				CCharacter* pChr = GetNearest(ms_PhysSize - 4.0f, m_Team);

				if (!pChr && m_Anim == 8)
				{
					pChr = GetNearest(ms_PhysSize - 4.0f, m_Team ^ 1);
					IsEnemy = true;
				}

				if (pChr)
				{
					if (m_Anim < 8 && m_AnimDecay == 0 && !IsEnemy)
					{
						m_Anim++;
						m_AnimDecay = 2;
						pOther->m_Anim = m_Anim;
						pOther->m_AnimDecay = m_AnimDecay + 1;
						pOther->m_Decay = m_AnimDecay + 1;
					}
					else if (m_Anim == 8 && m_AnimDecay == 0)
					{
						if (!pChr->GetPlayer()->m_Ported)
						{
							float IllProb = min((distance(pOther->m_Pos, m_Pos)) / 8000.0f * 0.2f, 0.2f);
							for (int i = 0; i < MAX_CLIENTS; i++)
							{
								if (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetCharacter() && (GameServer()->m_apPlayers[i]->GetCharacter()->HookedPlayer() == pChr->GetPlayer()->GetCID() || pChr->HookedPlayer() == i))
								{
									CCharacter* pHookedChr = GameServer()->m_apPlayers[i]->GetCharacter();
									pHookedChr->SetPos(vec2(pOther->m_Pos.x, pOther->m_Pos.y - 1));
									pHookedChr->SetCorePos(vec2(pOther->m_Pos.x, pOther->m_Pos.y - 1));
									pHookedChr->GetPlayer()->m_Ported = true;
									pHookedChr->SetHookState(-1);
									pHookedChr->SetHookedPlayer(-1);

									if (frandom() <= IllProb)
									{
										if (!pHookedChr->m_Ill)
										{
											pHookedChr->m_Ill = true;
											pHookedChr->m_IllInfecter = -1;
										}
									}
								}
							}

							pChr->SetPos(vec2(pOther->m_Pos.x, pOther->m_Pos.y - 1));
							pChr->SetCorePos(vec2(pOther->m_Pos.x, pOther->m_Pos.y - 1));
							pChr->GetPlayer()->m_Ported = true;
							pChr->SetHookState(-1);
							pChr->SetHookedPlayer(-1);

							if (frandom() <= IllProb)
							{
								if (!pChr->m_Ill)
								{
									pChr->m_Ill = true;
									pChr->m_IllInfecter = -1;
								}
							}

							m_Decay = 50;
							pOther->m_Decay = 50;
							m_Anim = 1;
							pOther->m_Anim = 1;

							GameServer()->CreatePlayerSpawn(pOther->m_Pos);
							GameServer()->CreateSound(m_Pos, SOUND_TELEPORT);
							GameServer()->CreateDeath(m_Pos, pChr->GetPlayer()->GetCID());
							GameServer()->CreateSound(pOther->m_Pos, SOUND_TELEPORT);
						}
						else
							m_Anim = 0;
					}
				}
				else
				{
					if (m_Anim > 0 && m_AnimDecay == 0)
					{
						m_Anim++;
						if (m_Anim == 9)
							m_Anim = 0;
						m_AnimDecay = 2;
						pOther->m_Anim = m_Anim;
						pOther->m_AnimDecay = m_AnimDecay + 1;
						pOther->m_Decay = m_AnimDecay + 1;
					}
				}
			}
		}
		else
		{
			if (m_AnimDecay == 0 && m_Anim > 0)
			{
				m_AnimDecay = 2;
				m_Anim++;
				if (m_Anim == 9)
					m_Anim = 0;
			}
		}
	} break;
	}
}

CCharacter* CBuilding::GetNearest(int MaxDist, int Team)
{
	CCharacter* CloseCharacters[MAX_CLIENTS];
	int Num = GameServer()->m_World.FindEntities(m_Pos, MaxDist, (CEntity**)CloseCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	int Disti = -1;
	float Dist = 10000000.0f;
	for (int i = 0; i < Num; i++)
	{
		if (!CloseCharacters[i])
			continue;

		if (CloseCharacters[i]->GetPlayer()->GetTeam() != Team || GameServer()->Collision()->IntersectLine(m_Pos, CloseCharacters[i]->GetPos(), 0x0, 0x0))
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

void CBuilding::HandlePower()
{
	if (m_Type == B_SPAWN)
	{
		m_Power = true;
		return;
	}

	bool Reactor = false;
	bool Power = false;

	for (int i = 0; i < ((CGameControllerNODES*)GameServer()->m_pController)->m_aBuildingsCount[m_Team]; i++)
	{
		CBuilding* pBuilding = ((CGameControllerNODES*)GameServer()->m_pController)->m_apBuildings[m_Team][i];
		if (pBuilding)
		{
			if (pBuilding->m_Type == B_REACTOR && pBuilding->m_Alive)
				Reactor = true;

			if (((pBuilding->m_Type == B_REACTOR && distance(pBuilding->m_Pos, m_Pos) < 750.0f) || (pBuilding->m_Type == B_REPEATER && distance(pBuilding->m_Pos, m_Pos) < 500.0f)) && pBuilding->m_Alive)
				Power = true;
		}
	}

	m_Power = Power && Reactor && !((CGameControllerNODES*)GameServer()->m_pController)->m_Emp;
}

bool CBuilding::IncreaseHealth(int Amount)
{
	if (m_Health >= m_MaxHealth)
		return false;

	int Gain = 2;
	if (m_HealthTick + 25 > Server()->Tick())
		Gain = 1;
	else
		m_HealthTick = Server()->Tick();

	m_Deconstruction = false;

	m_Health = clamp(m_Health + Gain, 0, m_MaxHealth);
	if (m_Health == m_MaxHealth && !m_Alive)
	{
		m_Alive = true;
		if (m_Type == B_REACTOR)
			GameServer()->CreateSound(m_Pos, SOUND_REACTOR);

		GameServer()->DoTeamSound(SOUND_CTF_GRAB_PL, m_Team);
	}

	return true;
}

bool CBuilding::TakeDamage(vec2 Force, int Dmg, int From, int PrevFrom, int Weapon)
{
	if (From < 0 && PrevFrom >= 0)
		From = PrevFrom;

	if (From >= 0 && GameServer()->m_apPlayers[From] && GameServer()->m_apPlayers[From]->GetTeam() == m_Team && !Config()->m_SvBuildingsFriendlyFire)
		return false;

	m_DamageTaken++;

	// create healthmod indicator
	if (Server()->Tick() < m_DamageTakenTick + 25 && !m_Deconstruction)
		GameServer()->CreateDamageInd(m_Pos, m_DamageTaken * 0.25f, Dmg);
	else if (!m_Deconstruction)
	{
		m_DamageTaken = 0;
		GameServer()->CreateDamageInd(m_Pos, 0, Dmg);
	}

	if (Dmg)
		m_Health -= Dmg;

	m_DamageTakenTick = Server()->Tick();
	if (m_DamageTick + (3 * Server()->TickSpeed()) < Server()->Tick())
	{
		m_DamageTick = Server()->Tick();

		if (m_Health < m_MaxHealth / 4)
		{
			if (m_Type == B_REACTOR)
				GameServer()->SendChat(-1, CHAT_TEAM, m_Team, "The reactor is badly damaged!");
		}
		else
		{
			if (m_Type == B_REACTOR)
				GameServer()->SendChat(-1, CHAT_TEAM, m_Team, "The reactor is under attack!");
		}
	}

	// do damage hit sound
	if (From >= 0 && GameServer()->m_apPlayers[From])
	{
		GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, CmaskOne(From));
		if (GameServer()->m_apPlayers[From]->GetTeam() != m_Team)
			GameServer()->m_apPlayers[From]->m_Rage = min(100, GameServer()->m_apPlayers[From]->m_Rage + Dmg * Config()->m_SvRageModifier);
		else
			GameServer()->m_apPlayers[From]->m_Rage = max(0, GameServer()->m_apPlayers[From]->m_Rage - Dmg * Config()->m_SvRageModifier);
	}

	// check for death
	if (m_Health <= 0)
	{
		// set attacker's face to happy (taunt!)
		if (From >= 0 && GameServer()->m_apPlayers[From])
		{
			CCharacter* pChr = GameServer()->m_apPlayers[From]->GetCharacter();
			if (pChr && GameServer()->m_apPlayers[From]->GetTeam() != m_Team)
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
		}

		Die(From, Weapon);

		return false;
	}

	return true;
}

void CBuilding::Die(int Killer, int Weapon)
{
	bool WasAlive = m_Alive;
	m_Alive = false;

	GameServer()->m_World.RemoveEntity(this);

	if (Killer != -5)
	{
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);

		if (m_Type == B_REACTOR && WasAlive && !m_Deconstruction)
			((CGameControllerNODES*)GameServer()->m_pController)->DoFallout(m_Pos);

		GameServer()->CreateDeath(m_Pos, -1);
		if (Killer != -4)
			GameServer()->CreateExplosion(m_Pos, -4, WEAPON_WORLD, 5);
	}

	if (Killer >= 0 && GameServer()->m_apPlayers[Killer] && GameServer()->m_apPlayers[Killer]->GetTeam() != m_Team)
	{
		CNetMsg_Sv_KillMsg Msg;
		Msg.m_Killer = Killer;
		Msg.m_Victim = -(m_Type + 1);
		Msg.m_Weapon = Weapon;
		Msg.m_ModeSpecial = 0;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
	}

	((CGameControllerNODES*)GameServer()->m_pController)->DestroyBuilding(this, (Killer == -5 ? true : false));
}

bool CBuilding::IsGrounded()
{
	if (GameServer()->Collision()->CheckPoint(m_Pos.x + GetProximityRadius() / 2, m_Pos.y + GetProximityRadius() / 2 + 5))
		return true;
	if (GameServer()->Collision()->CheckPoint(m_Pos.x - GetProximityRadius() / 2, m_Pos.y + GetProximityRadius() / 2 + 5))
		return true;
	return false;
}

void CBuilding::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient, m_Pos) || 
		(!GameServer()->GetPlayerChar(SnappingClient) && GameServer()->m_apPlayers[SnappingClient]->m_SelectSpawn && (m_Type != B_SPAWN || m_Team != GameServer()->m_apPlayers[SnappingClient]->GetTeam())))
		return;

	CNetObj_Building* pBuilding = static_cast<CNetObj_Building*>(Server()->SnapNewItem(NETOBJTYPE_BUILDING, GetID(), sizeof(CNetObj_Building)));
	if (!pBuilding)
		return;

	pBuilding->m_X = (int)m_Pos.x;
	pBuilding->m_Y = (int)m_Pos.y;

	if (!GameServer()->GetPlayerChar(SnappingClient) && GameServer()->m_apPlayers[SnappingClient]->m_SelectSpawn)
		pBuilding->m_Health = 0;
	else
		pBuilding->m_Health = 80 * m_Health / m_MaxHealth;

	pBuilding->m_Team = m_Team;
	pBuilding->m_Anim = m_Anim & 0xF;
	pBuilding->m_Anim2 = m_Anim >> 4;
	pBuilding->m_Type = m_Type;
	pBuilding->m_Power = m_Power;
	pBuilding->m_Angle = (int)((m_Angle + pi / 2) / (2 * pi) * MAX_INT);
	pBuilding->m_Alive = m_Alive;
	pBuilding->m_TeamFake = false;
}