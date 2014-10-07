#pragma once

#define MIN_PLAYER_NAME 1
#define MAX_PLAYER_NAME 16

#include <thread>
#include "CNetworkManager.h"
#include "NetworkCallback.h"
#include "CPlayerManager.h"

class CServer
{
public:
	static void Initialize(int iPort, int iLogLevel);
	static inline CPlayerManager* GetPlayerManager() { return m_pPlayerManager; };
	static inline CNetworkManager* GetNetworkManager() { return m_pNetworkManager; };
	static bool NameCheck(const char* szName, const size_t sLen);
	
private:
	static void StartGame();
	static void GameLoop();
	static void NetLoop();

	static CPlayerManager* m_pPlayerManager;
	static CNetworkManager* m_pNetworkManager;

};