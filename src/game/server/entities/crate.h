#ifndef GAME_SERVER_ENTITY_CRATE_H
#define GAME_SERVER_ENTITY_CRATE_H

#include <game/server/entity.h>
#include <generated/server_data.h>
#include <generated/protocol.h>

#include <game/gamecore.h>

class CCrate : public CEntity
{
private:

public:
	int m_Type;

	int64 m_CreationTick;

	bool m_Alive;
	bool m_Grounded;

	CCrate(CGameWorld* pGameWorld, int Type, vec2 Pos);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

	CCharacter* GetNearest(int MaxDist);

	void Die();
	void Destroy();

	bool Remove();
	bool IsGrounded();

	static const int ms_PhysSize = 28;
};

#endif