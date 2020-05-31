/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/demo.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>

#include <game/client/components/flow.h>
#include <game/client/components/effects.h>
#include <game/client/components/buildmenu.h>

#include <game/buildings_info.h>

#include "items.h"

void CItems::RenderProjectile(const CNetObj_Projectile *pCurrent, int ItemID)
{
	// get positions
	float Curvature = 0;
	float Speed = 0;
	if(pCurrent->m_Type == WEAPON_GRENADE)
	{
		Curvature = m_pClient->m_Tuning.m_GrenadeCurvature;
		Speed = m_pClient->m_Tuning.m_GrenadeSpeed;
	}
	else if(pCurrent->m_Type == WEAPON_SHOTGUN)
	{
		Curvature = m_pClient->m_Tuning.m_ShotgunCurvature;
		Speed = m_pClient->m_Tuning.m_ShotgunSpeed;
	}
	else if(pCurrent->m_Type == WEAPON_GUN)
	{
		Curvature = m_pClient->m_Tuning.m_GunCurvature;
		Speed = m_pClient->m_Tuning.m_GunSpeed;
	}

	static float s_LastGameTickTime = Client()->GameTickTime();
	if(m_pClient->m_Snap.m_pGameData && !(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
		s_LastGameTickTime = Client()->GameTickTime();
	float Ct = (Client()->PrevGameTick()-pCurrent->m_StartTick)/(float)SERVER_TICK_SPEED + s_LastGameTickTime;
	if(Ct < 0)
		return; // projectile haven't been shot yet

	vec2 StartPos(pCurrent->m_X, pCurrent->m_Y);
	vec2 StartVel(pCurrent->m_VelX/100.0f, pCurrent->m_VelY/100.0f);
	vec2 Pos = CalcPos(StartPos, StartVel, Curvature, Speed, Ct);
	vec2 PrevPos = CalcPos(StartPos, StartVel, Curvature, Speed, Ct-0.001f);


	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();

	RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[clamp(pCurrent->m_Type, 0, NUM_WEAPONS-1)].m_pSpriteProj);
	vec2 Vel = Pos-PrevPos;
	//vec2 pos = mix(vec2(prev->x, prev->y), vec2(current->x, current->y), Client()->IntraGameTick());


	// add particle for this projectile
	if(pCurrent->m_Type == WEAPON_GRENADE)
	{
		m_pClient->m_pEffects->SmokeTrail(Pos, Vel*-1);
		static float s_Time = 0.0f;
		static float s_LastLocalTime = Client()->LocalTime();

		if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		{
			const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
			if(!pInfo->m_Paused)
				s_Time += (Client()->LocalTime()-s_LastLocalTime)*pInfo->m_Speed;
		}
		else
		{
			if(m_pClient->m_Snap.m_pGameData && !(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
				s_Time += Client()->LocalTime()-s_LastLocalTime;
		}

		Graphics()->QuadsSetRotation(s_Time*pi*2*2 + ItemID);
		s_LastLocalTime = Client()->LocalTime();
	}
	else
	{
		m_pClient->m_pEffects->BulletTrail(Pos);

		if(length(Vel) > 0.00001f)
			Graphics()->QuadsSetRotation(angle(Vel));
		else
			Graphics()->QuadsSetRotation(0);

	}

	if (pCurrent->m_Rampage)
		m_pClient->m_pEffects->FireTrail(Pos, Vel * -1);

	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, 32, 32);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsSetRotation(0);
	Graphics()->QuadsEnd();
}

void CItems::RenderPickup(const CNetObj_Pickup *pPrev, const CNetObj_Pickup *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();
	vec2 Pos = mix(vec2(pPrev->m_X, pPrev->m_Y), vec2(pCurrent->m_X, pCurrent->m_Y), Client()->IntraGameTick());
	float Angle = 0.0f;
	float Size = 64.0f;
	const int c[] = {
		SPRITE_PICKUP_HEALTH,
		SPRITE_PICKUP_ARMOR,
		SPRITE_PICKUP_GRENADE,
		SPRITE_PICKUP_SHOTGUN,
		SPRITE_PICKUP_LASER,
		SPRITE_PICKUP_NINJA,
		SPRITE_PICKUP_GUN,
		SPRITE_PICKUP_HAMMER
		};
	RenderTools()->SelectSprite(c[pCurrent->m_Type]);

	switch(pCurrent->m_Type)
	{
	case PICKUP_GRENADE:
		Size = g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_VisualSize;
		break;
	case PICKUP_SHOTGUN:
		Size = g_pData->m_Weapons.m_aId[WEAPON_SHOTGUN].m_VisualSize;
		break;
	case PICKUP_LASER:
		Size = g_pData->m_Weapons.m_aId[WEAPON_LASER].m_VisualSize;
		break;
	case PICKUP_NINJA:
		m_pClient->m_pEffects->PowerupShine(Pos, vec2(96,18));
		Size *= 2.0f;
		Pos.x -= 10.0f;
		break;
	case PICKUP_GUN:
		Size = g_pData->m_Weapons.m_aId[WEAPON_GUN].m_VisualSize;
		break;
	case PICKUP_HAMMER:
		Size = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_VisualSize;
		break;
	}
	

	Graphics()->QuadsSetRotation(Angle);

	static float s_Time = 0.0f;
	static float s_LastLocalTime = Client()->LocalTime();
	float Offset = Pos.y/32.0f + Pos.x/32.0f;
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(!pInfo->m_Paused)
			s_Time += (Client()->LocalTime()-s_LastLocalTime)*pInfo->m_Speed;
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameData && !(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
			s_Time += Client()->LocalTime()-s_LastLocalTime;
 	}
	Pos.x += cosf(s_Time*2.0f+Offset)*2.5f;
	Pos.y += sinf(s_Time*2.0f+Offset)*2.5f;
	s_LastLocalTime = Client()->LocalTime();
	RenderTools()->DrawSprite(Pos.x, Pos.y, Size);
	Graphics()->QuadsEnd();
}

void CItems::RenderFlag(const CNetObj_Flag *pPrev, const CNetObj_Flag *pCurrent, const CNetObj_GameDataFlag *pPrevGameDataFlag, const CNetObj_GameDataFlag *pCurGameDataFlag)
{
	float Angle = 0.0f;
	float Size = 42.0f;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();

	if(pCurrent->m_Team == TEAM_RED)
		RenderTools()->SelectSprite(SPRITE_FLAG_RED);
	else
		RenderTools()->SelectSprite(SPRITE_FLAG_BLUE);

	Graphics()->QuadsSetRotation(Angle);

	vec2 Pos = mix(vec2(pPrev->m_X, pPrev->m_Y), vec2(pCurrent->m_X, pCurrent->m_Y), Client()->IntraGameTick());

	if(pCurGameDataFlag)
	{
		// make sure that the flag isn't interpolated between capture and return
		if(pPrevGameDataFlag &&
			((pCurrent->m_Team == TEAM_RED && pPrevGameDataFlag->m_FlagCarrierRed != pCurGameDataFlag->m_FlagCarrierRed) ||
			(pCurrent->m_Team == TEAM_BLUE && pPrevGameDataFlag->m_FlagCarrierBlue != pCurGameDataFlag->m_FlagCarrierBlue)))
			Pos = vec2(pCurrent->m_X, pCurrent->m_Y);

		// make sure to use predicted position if we are the carrier
		if(m_pClient->m_LocalClientID != -1 &&
			((pCurrent->m_Team == TEAM_RED && pCurGameDataFlag->m_FlagCarrierRed == m_pClient->m_LocalClientID) ||
			(pCurrent->m_Team == TEAM_BLUE && pCurGameDataFlag->m_FlagCarrierBlue == m_pClient->m_LocalClientID)))
			Pos = m_pClient->m_LocalCharacterPos;
	}

	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y-Size*0.75f, Size, Size*2);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
}


void CItems::RenderLaser(const struct CNetObj_Laser *pCurrent)
{
	vec2 Pos = vec2(pCurrent->m_X, pCurrent->m_Y);
	vec2 From = vec2(pCurrent->m_FromX, pCurrent->m_FromY);
	vec2 Dir = normalize(Pos-From);

	float Ticks = (Client()->GameTick() - pCurrent->m_StartTick) + Client()->IntraGameTick();
	float Ms = (Ticks/50.0f) * 1000.0f;
	float a = Ms / m_pClient->m_Tuning.m_LaserBounceDelay;
	a = clamp(a, 0.0f, 1.0f);
	float Ia = 1-a;

	vec2 Out, Border;

	Graphics()->BlendNormal();
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();

	//vec4 inner_color(0.15f,0.35f,0.75f,1.0f);
	//vec4 outer_color(0.65f,0.85f,1.0f,1.0f);

	// do outline
	vec4 OuterColor(0.075f, 0.075f, 0.25f, 1.0f);
	Graphics()->SetColor(OuterColor.r, OuterColor.g, OuterColor.b, 1.0f);
	Out = vec2(Dir.y, -Dir.x) * (7.0f*Ia);

	IGraphics::CFreeformItem Freeform(
			From.x-Out.x, From.y-Out.y,
			From.x+Out.x, From.y+Out.y,
			Pos.x-Out.x, Pos.y-Out.y,
			Pos.x+Out.x, Pos.y+Out.y);
	Graphics()->QuadsDrawFreeform(&Freeform, 1);

	// do inner
	vec4 InnerColor(0.5f, 0.5f, 1.0f, 1.0f);
	Out = vec2(Dir.y, -Dir.x) * (5.0f*Ia);
	Graphics()->SetColor(InnerColor.r, InnerColor.g, InnerColor.b, 1.0f); // center

	Freeform = IGraphics::CFreeformItem(
			From.x-Out.x, From.y-Out.y,
			From.x+Out.x, From.y+Out.y,
			Pos.x-Out.x, Pos.y-Out.y,
			Pos.x+Out.x, Pos.y+Out.y);
	Graphics()->QuadsDrawFreeform(&Freeform, 1);

	Graphics()->QuadsEnd();

	// render head
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
		Graphics()->QuadsBegin();

		int Sprites[] = {SPRITE_PART_SPLAT01, SPRITE_PART_SPLAT02, SPRITE_PART_SPLAT03};
		RenderTools()->SelectSprite(Sprites[Client()->GameTick()%3]);
		Graphics()->QuadsSetRotation(Client()->GameTick());
		Graphics()->SetColor(OuterColor.r, OuterColor.g, OuterColor.b, 1.0f);
		IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, 24, 24);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->SetColor(InnerColor.r, InnerColor.g, InnerColor.b, 1.0f);
		QuadItem = IGraphics::CQuadItem(Pos.x, Pos.y, 20, 20);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	Graphics()->BlendNormal();
}

void CItems::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_PROJECTILE)
		{
			RenderProjectile((const CNetObj_Projectile *)pData, Item.m_ID);
		}
		else if(Item.m_Type == NETOBJTYPE_PICKUP)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if(pPrev)
				RenderPickup((const CNetObj_Pickup *)pPrev, (const CNetObj_Pickup *)pData);
		}
		else if(Item.m_Type == NETOBJTYPE_LASER)
		{
			RenderLaser((const CNetObj_Laser *)pData);
		}
		else if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const void* pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if (pPrev)
				RenderBuilding((const CNetObj_Building*)pPrev, (const CNetObj_Building*)pData);
		}
		else if (Item.m_Type == NETOBJTYPE_CRATE)
		{
			RenderCrate((const CNetObj_Crate*)pData);
		}
	}

	// render flag
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_FLAG)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if (pPrev)
			{
				const void *pPrevGameDataFlag = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_GAMEDATAFLAG, m_pClient->m_Snap.m_GameDataFlagSnapID);
				RenderFlag(static_cast<const CNetObj_Flag *>(pPrev), static_cast<const CNetObj_Flag *>(pData),
							static_cast<const CNetObj_GameDataFlag *>(pPrevGameDataFlag), m_pClient->m_Snap.m_pGameDataFlag);
			}
		}
		else if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const void* pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if (pPrev)
				RenderBuilding(static_cast<const CNetObj_Building*>(pPrev), static_cast<const CNetObj_Building*>(pData), 1);
		}
	}

	for (int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void* pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const void* pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if (pPrev)
				RenderBuilding(static_cast<const CNetObj_Building*>(pPrev), static_cast<const CNetObj_Building*>(pData), 2);
		}
	}
}

void CItems::RenderBuilding(const CNetObj_Building* pPrev, const CNetObj_Building* pCurrent, int Rendering)
{
	float Angle = 0.0f;
	float Size = 64.0f;
	float Width = 1;
	float Height = 1;
	float PosX = 0;
	float PosY = 0;
	float Scale = 0.5f;
	vec2 Pos;

	Graphics()->BlendNormal();

	int Type = pCurrent->m_Type;
	int t = pCurrent->m_Team & 0x1;
	int Anim = pCurrent->m_Anim;
	int Anim2 = pCurrent->m_Anim2;
	bool Alive = pCurrent->m_Alive;
	bool TeamFake = pCurrent->m_TeamFake;
	Angle = (float)pCurrent->m_Angle / MAX_INT * 2 * pi - pi / 2;
	int Health = pCurrent->m_Health;
	int Powered = pCurrent->m_Power;

	Pos = vec2(pCurrent->m_X, pCurrent->m_Y);
	PosX = Pos.x;
	PosY = Pos.y + Size / 4;

	if (!Alive)
		PosY += (80 - Health) * Size / 160;

	int Sprite = SPRITE_BUILDING_REACTOR_RED;
	if (t == 0)
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDING_REACTOR_RED + 2 * Type].m_Id);
	else
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDING_REACTOR_BLUE + 2 * Type].m_Id);

	switch (Type)
	{
	case B_REACTOR:  Sprite = (SPRITE_BUILDING_REACTOR_RED + 2 * Anim); Width = 3; Height = 4; break;
	case B_SPAWN:  Sprite = (SPRITE_BUILDING_SPAWN_RED + 2 * Anim); Width = 3; Height = 4; break;
	case B_AMMO_SHOTGUN:  Sprite = (SPRITE_BUILDING_AMMO1_RED + 2 * Anim); Width = 2; Height = 2; break;
	case B_HEALTH:  Sprite = (SPRITE_BUILDING_HEALTH_RED + 2 * Anim); Width = 2; Height = 2; break;
	case B_REPEATER:  Sprite = (SPRITE_BUILDING_REPEATER_RED + 2 * Anim); Width = 2; Height = 3; break;
	case B_TURRET_GUN:  Sprite = (SPRITE_BUILDING_TURRET1_RED); Width = 3; Height = 3; break;
	case B_SHIELD:  Sprite = (SPRITE_BUILDING_SHIELD_RED + 2 * Anim); Width = 3; Height = 3; break;
	case B_AMMO_GRENADE: Sprite = (SPRITE_BUILDING_AMMO2_RED + 2 * Anim); Width = 2; Height = 2; break;
	case B_TELEPORT: Sprite = (SPRITE_BUILDING_TELEPORT_RED + 2 * Anim); Width = 3; Height = 4; break;
	case B_ARMOR: Sprite = (SPRITE_BUILDING_ARMOR_RED + 2 * Anim); Width = 2; Height = 2; break;
	case B_AMMO_LASER: Sprite = (SPRITE_BUILDING_AMMO3_RED + 2 * Anim); Width = 2; Height = 2; break;
	case B_TURRET_SHOTGUN: Sprite = (SPRITE_BUILDING_TURRET2_RED); Width = 3; Height = 3; break;
	}

	if (Rendering == 0)
	{
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(Sprite);
		Graphics()->QuadsSetRotation(0);
		IGraphics::CQuadItem QuadItem(PosX, PosY - (Height * Size * Scale / 2), Width * Size * Scale, Height * Size * Scale);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();

		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_TURRETS].m_Id);

		if (Type == B_TURRET_GUN || Type == B_TURRET_SHOTGUN)
		{
			Graphics()->QuadsBegin();
			if (Type == B_TURRET_GUN)
				RenderTools()->SelectSprite(SPRITE_TURRET1_HEAD, (Angle + pi / 2 < pi ? SPRITE_FLAG_FLIP_Y : 0));
			else
				RenderTools()->SelectSprite(SPRITE_TURRET2_HEAD, (Angle + pi / 2 < pi ? SPRITE_FLAG_FLIP_Y : 0));

			Width = 7;
			int tHeight = 4;
			Graphics()->QuadsSetRotation(Angle + pi);
			IGraphics::CQuadItem QuadItem(PosX + cos(Angle) * 14, PosY - 90 + 2, Width * Size * Scale / 2, tHeight * Size * Scale / 2);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}

		if (m_pClient->m_Snap.m_pLocalInfo && ((!TeamFake && m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team == t) || (TeamFake && TEAM_BLUE == t)) && (Type == B_REACTOR || (Type == B_REPEATER && Powered)) && m_pClient->m_pBuildMenu->Active())
		{
			Graphics()->TextureClear();
			Graphics()->QuadsBegin();
			Graphics()->SetColor(0.5f, 0.5f, 0.2f, 0.3f);
			int r = 750;
			if (Type == B_REPEATER)
				r = 500;
			RenderTools()->DrawRoundRectExt(PosX - r, PosY - r, 2 * r, 2 * r, r, CUI::CORNER_ALL);
			Graphics()->QuadsEnd();
		}

		if (Type == B_SPAWN && Anim2 > 0)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDING_SPAWN_RED + t].m_Id);
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(SPRITE_BUILDING_SPAWN_RED + 2 * 6 + (Anim2) * 2);
			Graphics()->QuadsSetRotation(0);
			IGraphics::CQuadItem QuadItem(PosX, PosY - (Height * Size * Scale / 2) + 2, Width * Size * Scale, Height * Size * Scale);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
	}
	else if (Rendering == 1)
	{
		if (Type == B_SHIELD && Anim2 > 0)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDING_SHIELD_HIT_RED + t].m_Id);
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(SPRITE_BUILDING_SHIELD_HIT_RED + (Anim2 - 1) * 2);
			Graphics()->QuadsSetRotation(0);
			int tHeight = 3;
			int tWidth = 3;
			float tScale = 2.0f;
			IGraphics::CQuadItem QuadItem(PosX, PosY, tWidth * Size * tScale, tHeight * Size * tScale);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
	}
	else if (Rendering == 2)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_NOPOWER].m_Id);

		if (!Powered && Client()->GameTick() % 50 >= 25)
		{
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(SPRITE_NOPOWER);
			Graphics()->QuadsSetRotation(0);
			Width = 2;
			RenderTools()->DrawSprite(PosX, PosY - (Height * Size * Scale) - 40, 2 * Size * Scale);
			Graphics()->QuadsEnd();
		}

		if (Type == B_SHIELD)
		{
			if (m_pClient->m_Snap.m_pLocalInfo && (((!TeamFake && m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team == t) || (TeamFake && TEAM_BLUE == t)) || m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team == -1))
			{
				Graphics()->TextureClear();
				Graphics()->QuadsBegin();
				Graphics()->SetColor(0, 0, 0, 0.5f);
				RenderTools()->DrawRoundRectExt(PosX - 42, PosY - (Height * Size * Scale) - 5, 84, 14, 3);
				int Shield = Anim + 1;
				Graphics()->SetColor(1.0f, 1.0f, 0, 0.8f);
				RenderTools()->DrawRoundRectExt(PosX - 40, PosY - (Height * Size * Scale) - 3, Shield * 8, 10, 3);
				Graphics()->QuadsEnd();
			}
		}

		if (Health != 0)
		{
			if (m_pClient->m_Snap.m_pLocalInfo && (((!TeamFake && m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team == t) || (TeamFake && TEAM_BLUE == t)) || m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team == -1))
			{
				Graphics()->TextureClear();
				Graphics()->QuadsBegin();
				Graphics()->SetColor(0, 0, 0, 0.5f);
				RenderTools()->DrawRoundRectExt(PosX - 42, PosY - (Height * Size * Scale) - 20, 84, 14, 3);
				if (Health > 40)
					Graphics()->SetColor(1.0f - (Health - 40.0f) / 40.0f, 1.0f, 0, 0.8f);
				else
					Graphics()->SetColor(1.0f, Health / 40.0f, 0, 0.8f);
				RenderTools()->DrawRoundRectExt(PosX - 40, PosY - (Height * Size * Scale) - 18, Health, 10, 3);
				Graphics()->QuadsEnd();
			}

			if (Health < 30 && (Client()->GameTick() % max((Health / 3), 1) == 0) && Alive)
				m_pClient->m_pEffects->Smoke(vec2(PosX, PosY - 20), vec2(2, -10));
		}
	}
}

void CItems::RenderCrate(const struct CNetObj_Crate* pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CRATE].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SPRITE_CRATE2);
	if (!pCurrent->m_Grounded)
		Graphics()->QuadsSetRotation(sin((Client()->GameTick() % 50) * 2 * 3.141593 / Client()->GameTickSpeed()) * 0.2f);
	IGraphics::CQuadItem QuadItem(pCurrent->m_X, pCurrent->m_Y - 38.0f, 64.0f, 128.0f);
	Graphics()->QuadsDraw(&QuadItem, 1);

	if (!pCurrent->m_Grounded)
	{
		RenderTools()->SelectSprite(SPRITE_PARACHUTE);
		Graphics()->QuadsSetRotation(0);
		QuadItem = IGraphics::CQuadItem(pCurrent->m_X, pCurrent->m_Y - 70.0f, 96.0f, 96.0f);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}

	Graphics()->QuadsEnd();
}