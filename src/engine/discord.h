/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_DISCORD_H
#define ENGINE_DISCORD_H
#include <memory>
#include <string>
#include <discord.h>

#include "kernel.h"
#include "message.h"
#include "graphics.h"
#include <engine/shared/protocol.h>

class IDiscord : public IInterface
{
	MACRO_INTERFACE("discord", 0)
protected:

public:

	struct DiscordState {
		bool m_UserFound;

		discord::User m_CurrentUser;

		std::unique_ptr<discord::Core> m_Core;
	};

	DiscordState m_DiscordState;

	virtual void Init() = 0;
	virtual void OnTick() = 0;

	virtual discord::User CurrentUser() { return m_DiscordState.m_CurrentUser; };

	virtual bool UserFound() { return m_DiscordState.m_Core && m_DiscordState.m_UserFound; }
};

IDiscord* CreateDiscordWrapper();
#endif