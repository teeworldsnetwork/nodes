#ifndef GAME_SERVER_ENTITY_BUILDING_H
#define GAME_SERVER_ENTITY_BUILDING_H

#include <game/server/entity.h>

class CBuilding : public CEntity
{
	int m_Hit;
	int m_Anim;
	int m_Type;
	int m_Team;
	int m_Owner;
	int m_Decay;
	int m_Armor;
	int m_Health;
	int m_AnimDecay;
	int m_MaxHealth;
	int m_SpawnAnim;
	int m_ReloadTime;
	int m_MaxAimRange;
	int m_DamageTaken;
	int m_VeteranShots;
	int m_SpawnAnimDecay;
	int m_DamageTakenTick;

	int64 m_HealthTick;
	int64 m_DamageTick;

	float m_Angle;
	float m_Width;
	float m_Height;
	float m_VeteranTTLBonus;

	bool m_Free;
	bool m_Alive;
	bool m_Power;
	bool m_Deconstruction;

	void Destroy();
	void HandlePower();
	void Die(int Killer, int Weapon);

public:
	static const int ms_PhysSize = 28;

	CBuilding(CGameWorld* pGameWorld, int Type, vec2 Pos, int Team, int Owner, int Health, bool Alive = false, bool Free = false);

	virtual void Tick();
	virtual void Reset();
	virtual void Snap(int SnappingClient);

	int Type() { return m_Type; }
	int Team() { return m_Team; }
	int Owner() { return m_Owner; }
	int Health() { return m_Health; }
	int MaxHealth() { return m_MaxHealth; }

	float Width() { return m_Width; }
	float Height() { return m_Height; }

	bool IsGrounded();
	bool Alive() { return m_Alive; }
	bool Power() { return m_Power; }
	bool IncreaseHealth(int Amount);
	bool WasFree() { return m_Free; }
	bool Deconstruction() { return m_Deconstruction; }
	bool TakeDamage(vec2 Force, int Dmg, int From, int PrevFrom, int Weapon);

	CCharacter* GetNearest(int MaxDist, int Team);

	void SetOwner(int Owner) { m_Owner = Owner; }
	void SetSpawnAnim(int SpawnAnim) { m_SpawnAnim = SpawnAnim; }
	void SetDeconstruction(bool Status) { m_Deconstruction = Status; }
	void SetSpawnAnimDecay(int SpawnAnimDecay) { m_SpawnAnimDecay = SpawnAnimDecay; }
};

#endif