#pragma once

#include <map>
#include <mutex>
#include "CNetworkManager.h"
#include "CPlayer.h"

class CPlayerManager
{
public:
	CPlayerManager(unsigned char ucMaxPlayers);
	CPlayer* Add(libwebsocket* pLws, std::string strNickname);
	void Remove(unsigned char ucId);
	void Remove(libwebsocket* pLws);
	CPlayer* GetPlayerFromLws(libwebsocket* pLws);
	unsigned char GetPlayerIdFromLws(libwebsocket* pLws);
	unsigned char GetPlayerId(const CPlayer* pPlayer);
	CPlayer* GetPlayerByName(std::string strName);
	CPlayer* GetPlayerByName(const char* szName);
	void SendInitialInfo(CPlayer* pPlayer);
	void BroadcastTickInfo();
	void BroadcastJoin(CPlayer* pPlayer);
	void BroadcastLeave(unsigned char ucId);
	void Broadcast(const unsigned char* pBuf, const size_t sLen, const CPlayer* pPlayerToIgnore = NULL);

	inline CPlayer* GetPlayerById(unsigned char ucId) { LockMutex(); CPlayer* pPlayer = m_ppPlayers[ucId]; UnlockMutex(); return pPlayer; };
	inline CPlayer** GetPlayerPool() { LockMutex(); CPlayer** ppPlayers = m_ppPlayers; UnlockMutex(); return ppPlayers; };
	inline bool PlayerExists(libwebsocket* pLws) { return GetPlayerIdFromLws(pLws) != 0xFF; };
	inline unsigned char GetPlayers() { LockMutex(); unsigned char ucPlayers = m_ucPlayers; UnlockMutex(); return ucPlayers; };
	inline unsigned char GetMaxPlayers() { LockMutex(); unsigned char ucMaxPlayers = m_ucMaxPlayers; UnlockMutex(); return ucMaxPlayers; };
	inline std::mutex& GetMutex() { return m_mutex; };
	inline void LockMutex() { m_mutex.lock(); };
	inline void UnlockMutex() { m_mutex.unlock(); };

private:
	unsigned char m_ucMaxPlayers;
	unsigned char m_ucPlayers;
	CPlayer** m_ppPlayers;
	std::mutex m_mutex;

};