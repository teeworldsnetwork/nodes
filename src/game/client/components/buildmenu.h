#ifndef GAME_CLIENT_COMPONENTS_BUILDMENU_H
#define GAME_CLIENT_COMPONENTS_BUILDMENU_H
#include <base/vmath.h>
#include <game/client/component.h>

class CBuildMenu : public CComponent
{
	bool m_WasActive;
	bool m_Active;

	vec2 m_SelectorMouse;

	int m_SelectedBuilding;

	static void ConBuild(IConsole::IResult* pResult, void* pUserData);
	static void ConKeyBuild(IConsole::IResult* pResult, void* pUserData);

public:
	CBuildMenu();

	virtual void OnReset();
	virtual void OnRender();
	virtual void OnRelease();
	virtual void OnConsoleInit();
	virtual bool OnMouseMove(float x, float y);

	void Build(int Building);

	bool Active() { return m_Active; }
};

#endif