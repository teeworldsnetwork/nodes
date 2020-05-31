/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_DISCORD_H
#define ENGINE_CLIENT_DISCORD_H

class CDiscord : public IDiscord
{
	class IClient* m_pClient;

	struct
	{
		int64 m_LastChange;
		int64 m_LastStateChange;

		int m_LastState;
	} m_ActivityData;

	void HandleActivity();
	void OnCurrentUserUpdate();
	void OnActivityJoin(const char* pSecret);
	void OnActivitySpectate(const char* pSecret);
	void OnActivityJoinRequest(discord::User const& User);
	void OnLog(discord::LogLevel Level, const char* pMessage);
	void OnActivityInvite(discord::ActivityActionType ActivityActionType, discord::User const& User, discord::Activity const& Activity);

public:

	CDiscord();

	class IClient* Client() { return m_pClient; }

	virtual void Init();
	virtual void OnTick();
};
#endif