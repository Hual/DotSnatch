#include "CServer.h"

CPlayerManager* CServer::m_pPlayerManager = NULL;
CNetworkManager* CServer::m_pNetworkManager = NULL;

void CServer::Initialize(int iPort, int iLogLevel)
{
	lws_set_log_level(iLogLevel, lwsl_emit_syslog);

	m_pPlayerManager = new CPlayerManager(50);
	m_pNetworkManager = new CNetworkManager(iPort, 50, NetworkCallback::ClientConnectCallback, NetworkCallback::ClientDisconnectCallback, NetworkCallback::ReceiveCallback);
	StartGame();
}

bool CServer::NameCheck(const char* szName, const size_t sLen)
{
	for (unsigned int i = 2; i < sLen; ++i)
		if (!isalnum(szName[i]) && szName[i] != '_')
			return false;

	return true;
}

void CServer::StartGame()
{
	std::thread t1(GameLoop);
	std::thread t2(NetLoop);
	
	t1.join();
	t2.join();
}

void CServer::NetLoop()
{
	printf("network loop\n");
	m_pNetworkManager->Listen();
}

void CServer::GameLoop()
{
	printf("game loop\n");
	while (true)
	{
		CPlayer** players = m_pPlayerManager->GetPlayerPool();

		for (unsigned char i = 0; i < m_pPlayerManager->GetPlayers(); ++i)
		{
			if (!players[i] || !players[i]->IsSpawned())
				continue;

			players[i]->Step();
		}
		m_pPlayerManager->BroadcastTickInfo();

		std::this_thread::sleep_for(std::chrono::milliseconds(80));
	}
}
