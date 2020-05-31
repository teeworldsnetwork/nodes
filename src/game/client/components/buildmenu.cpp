#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/ui.h>
#include <game/client/render.h>

#include <game/buildings_info.h>
#include "buildmenu.h"

CBuildMenu::CBuildMenu()
{
	OnReset();
}

void CBuildMenu::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_SelectedBuilding = -1;
}

void CBuildMenu::ConKeyBuild(IConsole::IResult* pResult, void* pUserData)
{
	CBuildMenu* pSelf = (CBuildMenu*)pUserData;
	if (!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
		pSelf->m_Active = pResult->GetInteger(0) != 0;
}

void CBuildMenu::ConBuild(IConsole::IResult* pResult, void* pUserData)
{
	((CBuildMenu*)pUserData)->Build(pResult->GetInteger(0));
}

void CBuildMenu::OnConsoleInit()
{
	Console()->Register("+buildmenu", "", CFGFLAG_CLIENT, ConKeyBuild, this, "Open building selector");
	Console()->Register("build", "i", CFGFLAG_CLIENT, ConBuild, this, "Build building");
}

void CBuildMenu::OnRelease()
{
	m_Active = false;
}

bool CBuildMenu::OnMouseMove(float x, float y)
{
	if (!m_Active)
		return false;

	UI()->ConvertMouseMove(&x, &y);
	m_SelectorMouse += vec2(x, y);
	return true;
}

void CBuildMenu::OnRender()
{
	if (!m_Active)
	{
		if (m_WasActive && m_SelectedBuilding != -1)
			Build(m_SelectedBuilding);
		m_WasActive = false;
		return;
	}

	if (m_pClient->m_Snap.m_SpecInfo.m_Active)
	{
		m_Active = false;
		m_WasActive = false;
		return;
	}

	m_WasActive = true;

	int dx = 96;

	if (m_SelectorMouse.x < -191 - dx)
		m_SelectorMouse.x = -191 - dx;

	if (m_SelectorMouse.x > 191 - dx)
		m_SelectorMouse.x = 191 - dx;

	if (abs((int)m_SelectorMouse.y) > 143)
		m_SelectorMouse.y = sign(m_SelectorMouse.y) * 143;

	int Col = (m_SelectorMouse.x + 192 + dx) / 96;
	int Row = (m_SelectorMouse.y + 144) / 96;

	m_SelectedBuilding = Row * 4 + Col;

	float tw = TextRender()->TextWidth(0, 24, "Tech 3 ", -1, -1);

	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	Graphics()->BlendNormal();
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0, 0, 0, 0.5f);

	int Height = 96;
	int Width = 96;
	int Left = Screen.w / 2 - 192 - dx;

	RenderTools()->DrawRoundRectExt(Left, Screen.h / 2 - Height - Height / 2, 4 * Width, Height - 1, 17.0f, 0);
	if (m_pClient->m_aTechLevel[m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team] < 2)
		Graphics()->SetColor(1.0f, 0, 0, 0.5f);

	RenderTools()->DrawRoundRectExt(Left, Screen.h / 2 - Height / 2, 4 * Width, Height - 2, 17.0f, 0);
	if (m_pClient->m_aTechLevel[m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team] < 3)
		Graphics()->SetColor(1.0f, 0, 0, 0.5f);

	RenderTools()->DrawRoundRectExt(Left, Screen.h / 2 + Height / 2, 4 * Width, Height, 17.0f, 0);
	Graphics()->SetColor(0, 0, 0, 0.5f);

	RenderTools()->DrawRoundRectExt(Left + 4 * Width + 2, Screen.h / 2 - Height - Height / 2, 2 * Width + Width / 2, 3 * Height, 17.0f, 0);
	Graphics()->QuadsEnd();

	TextRender()->Text(0, Left - tw, Screen.h / 2 - 144 + 48 - 18, 24, "Tech 1", -1);
	TextRender()->Text(0, Left - tw, Screen.h / 2 - 18, 24, "Tech 2", -1);
	TextRender()->Text(0, Left - tw, Screen.h / 2 + 48 + 48 - 18, 24, "Tech 3", -1);

	TextRender()->Text(0, Left + 4 * Width + 15, Screen.h / 2 - 144 + 7, 14, aBuildingsInfo[m_SelectedBuilding].m_pName, -1);

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Price: %d BP\n\n%s", aBuildingsInfo[m_SelectedBuilding].m_Price, aBuildingsInfo[m_SelectedBuilding].m_pDesc);
	TextRender()->Text(0, Left + 4 * Width + 15, Screen.h / 2 - 144 + 38, 14, aBuf, -1);

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDMENU].m_Id);
	Graphics()->QuadsBegin();

	for (int i = 0; i < 12; i++)
	{
		float Angle = 2 * pi * i / 12.0f;
		if (Angle > pi)
			Angle -= 2 * pi;

		int Row = i / 4;
		int Col = i % 4;

		bool Selected = m_SelectedBuilding == i && (Row < m_pClient->m_aTechLevel[m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team]);
		float Size = Selected ? 70 + 10 * sin((Client()->GameTick() % 75) / 75.0f * 2 * pi) : 64;
		float NudgeX = -144 - dx + 96 * Col;
		float NudgeY = -96 + 96 * Row;

		RenderTools()->SelectSprite(SPRITE_E_REACTOR + i);
		IGraphics::CQuadItem QuadItem(Screen.w / 2 + NudgeX, Screen.h / 2 + NudgeY, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}

	Graphics()->QuadsEnd();

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1, 1, 1, 1);
	IGraphics::CQuadItem QuadItem(m_SelectorMouse.x + Screen.w / 2, m_SelectorMouse.y + Screen.h / 2, 24, 24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

void CBuildMenu::Build(int Building)
{
	CNetMsg_Cl_BuildBuilding Msg;
	Msg.m_Building = Building;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}