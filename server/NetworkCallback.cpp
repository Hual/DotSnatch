#include "NetworkCallback.h"
#include <locale>
#include <stdio.h>

WEBSOCK_CALLBACK_RETURN NetworkCallback::ClientConnectCallback(WEBSOCK_CALLBACK_ARGS)
{
	if (CServer::GetPlayerManager()->PlayerExists(pId))
		return false;

	unsigned char data[1] = { PacketHeader::AUTH_REQUEST };

	CServer::GetNetworkManager()->Send(pId, data, sizeof(data));

	printf("client connected!\n");
	return true;
}

WEBSOCK_CALLBACK_RETURN NetworkCallback::ClientDisconnectCallback(WEBSOCK_CALLBACK_ARGS)
{
	if (CServer::GetPlayerManager()->PlayerExists(pId))
		CServer::GetPlayerManager()->Remove(pId);

	printf("client disconnected! players online: %u\n", CServer::GetPlayerManager()->GetPlayers());
	return true;
}

WEBSOCK_CALLBACK_RETURN NetworkCallback::ReceiveCallback(WEBSOCK_CALLBACK_ARGS)
{
	if (sLen)
	{
		switch (pData[0])
		{
		case PacketHeader::AUTH_RESPONSE:
		{
			const char* szNickname = NULL;

			if ((sLen - 2 == pData[1]) && (pData[1] > MIN_PLAYER_NAME) && (pData[1] < MAX_PLAYER_NAME) && CServer::NameCheck((szNickname = (const char*)&pData[2]), pData[1]))
			{
				if (CServer::GetPlayerManager()->GetPlayerByName(szNickname))
					RETURN_ERROR(pId, "Name taken.", 11);

				CPlayer* pPlayer = CServer::GetPlayerManager()->Add(pId, std::string(szNickname));

				CServer::GetPlayerManager()->SendInitialInfo(pPlayer);

				printf("got auth response, creating player: %s. players online: %u\n", szNickname, CServer::GetPlayerManager()->GetPlayers());
			}
			else
			{
				printf("no auth response, closing connection\n");
				RETURN_ERROR(pId, "Invalid name.", 13);
			}

			break;
		}
		case PacketHeader::INITIAL_INFO_RESPONSE:
		{
			CPlayer* pPlayer = CServer::GetPlayerManager()->GetPlayerFromLws(pId);

			if (!pPlayer || pPlayer->GetAuthPhase() != AuthPhase::AWAITING_INITIAL_INFO_RESPONSE)
				RETURN_ERROR(pId, "Improper auth!", 14);
			
			pPlayer->SetAuthPhase(AuthPhase::READY);

			pPlayer->Spawn(15, 15, 3);

			printf("player initialization done for player %s\n", pPlayer->GetNickname().c_str());

			break;
		}
		case PacketHeader::DIRECTION_CHANGE:
		{
			CPlayer* pPlayer = CServer::GetPlayerManager()->GetPlayerFromLws(pId);

			if (!pPlayer || pPlayer->GetAuthPhase() != AuthPhase::READY || !pPlayer->IsSpawned() || sLen != 2 || pData[1] >= SnakeDirection::NONE)
				RETURN_ERROR(pId, "Improper packet!", 16);

			if (pPlayer->GetDirection() / 2 == pData[1] / 2)
				break;

			pPlayer->SetDirection((SnakeDirection)pData[1]);

			break;
		}
		}
	}
	return true;
}
