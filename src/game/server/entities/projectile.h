/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_PROJECTILE_H
#define GAME_SERVER_ENTITIES_PROJECTILE_H

enum
{
	PLAYER_TURRET_SHOTGUN = -4,
	PLAYER_TURRET_GUN = -3,
	PLAYER_TEAM_BLUE = -2,
	PLAYER_TEAM_RED = -1
};

class CProjectile : public CEntity
{
public:
	CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon, bool Rampage = false, int PrevOwner = -1);

	vec2 GetPos(float Time);
	void FillInfo(CNetObj_Projectile *pProj);

	int GetOwner() const { return m_Owner; }
	void LoseOwner();

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int Owner() { return m_Owner; }

	vec2 GetRealPos() { return m_Pos; }

	int m_Bounce;
	int m_Weapon;
	int m_PrevOwner;
	int m_StartTick;

	vec2 m_InitPos;
	vec2 m_Direction;

private:
	int m_LifeSpan;
	int m_Owner;
	int m_OwnerTeam;
	int m_Type;
	int m_Damage;
	int m_SoundImpact;

	float m_Force;

	bool m_Explosive;
	bool m_Rampage;
};

#endif
