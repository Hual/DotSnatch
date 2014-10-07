#include "CPlayerManager.h"
#include "CServer.h"

#include <string.h>
#if !defined WIN32 && !defined _WIN32
	#include <strings.h>
	#define _stricmp strcasecmp
#endif

CPlayerManager::CPlayerManager(unsigned char ucMaxPlayers)
{
	m_ucMaxPlayers = ucMaxPlayers;
	m_ppPlayers = (CPlayer**)calloc(ucMaxPlayers, sizeof(CPlayer*));
	m_ucPlayers = 0;
}

CPlayer* CPlayerManager::Add(libwebsocket* pLws, std::string strNickname)
{
	//LockMutex();

	CPlayer* pPlayer = new CPlayer(pLws, strNickname);

	for (unsigned char i = 0; i < m_ucMaxPlayers; ++i)
	{
		if (!m_ppPlayers[i])
		{
			m_ppPlayers[i] = pPlayer;
			break;
		}
	}

	++m_ucPlayers;

	BroadcastJoin(pPlayer);

	//UnlockMutex();

	return pPlayer;
}

void CPlayerManager::Remove(unsigned char ucId)
{
	//LockMutex();
	BroadcastLeave(ucId);

	--m_ucPlayers;

	delete m_ppPlayers[ucId];
	m_ppPlayers[ucId] = 0;

	//UnlockMutex();
}

void CPlayerManager::Remove(libwebsocket* pLws)
{
	Remove(GetPlayerIdFromLws(pLws));
}

CPlayer* CPlayerManager::GetPlayerFromLws(libwebsocket* pLws)
{
	unsigned char ucId = GetPlayerIdFromLws(pLws);

	return ucId == 0xFF ? NULL : m_ppPlayers[ucId];
}

unsigned char CPlayerManager::GetPlayerIdFromLws(libwebsocket* pLws)
{
	//LockMutex();
	unsigned char ucResult = 0xFF;

	for (unsigned char i = 0; i < m_ucMaxPlayers; ++i)
		if (m_ppPlayers[i] && m_ppPlayers[i]->GetLws() == pLws)
		{
			ucResult = i;
			break;
		}

	//UnlockMutex();
	return ucResult;
}

CPlayer* CPlayerManager::GetPlayerByName(std::string strName)
{
	return GetPlayerByName(strName.c_str());
}

CPlayer* CPlayerManager::GetPlayerByName(const char* szName)
{
	//LockMutex();

	for (unsigned char i = 0; i < m_ucMaxPlayers; ++i)
	{
		if (m_ppPlayers[i] && !_stricmp(m_ppPlayers[i]->GetNickname().c_str(), szName))
			return m_ppPlayers[i];

	}

	//UnlockMutex();
	return NULL;
}

unsigned char CPlayerManager::GetPlayerId(const CPlayer* pPlayer)
{
	unsigned char ucResult = 0xFF;

	for (unsigned char i = 0; i < m_ucMaxPlayers; ++i)
		if (m_ppPlayers[i] && m_ppPlayers[i] == pPlayer)
		{
			ucResult = i;
			break;
		}

	return ucResult;
}

void CPlayerManager::SendInitialInfo(CPlayer* pPlayer)
{
	std::vector<unsigned char> initialInfo = std::vector<unsigned char>();
	initialInfo.push_back(PacketHeader::INITIAL_INFO_REQUEST);
	initialInfo.push_back(m_ucPlayers);
	unsigned int accumulator;

	//LockMutex();

	for (unsigned char i = 0; i < m_ucMaxPlayers; ++i)
	{
		register CPlayer* pPlayer = m_ppPlayers[i];
		
		if (pPlayer)
		{
			initialInfo.push_back(i); // id
			initialInfo.push_back(pPlayer->GetNickname().length()); // nickname length

			accumulator = initialInfo.size();

			initialInfo.resize(accumulator + pPlayer->GetNickname().length()*sizeof(char)); // free memory for nickname
			memcpy(&initialInfo[accumulator], pPlayer->GetNickname().c_str(), pPlayer->GetNickname().length()*sizeof(char)); // copy nickname

			if (pPlayer->IsSpawned())
			{
				initialInfo.push_back(true); // spawned

				accumulator = initialInfo.size();
				unsigned int uiElementCount = pPlayer->GetElementCount();

				initialInfo.resize(accumulator + sizeof(unsigned int)+(uiElementCount*sizeof(unsigned char))*2); // free memory for elements
				*(unsigned int*)&initialInfo[accumulator] = uiElementCount; // add element count
				accumulator += sizeof(unsigned int); // increase accumulator
				
				for (unsigned int j = 0; j < uiElementCount; ++j)
				{
					initialInfo[accumulator + 2 * j] = pPlayer->GetElement(j).x; // element x
					initialInfo[accumulator + 2 * j + 1] = pPlayer->GetElement(j).y; // element y
				}
			}
			else
				initialInfo.push_back(false); // not spawned

		}
	}

	//UnlockMutex();

	pPlayer->SendPacket(&initialInfo[0], initialInfo.size()*sizeof(unsigned char));
}

void CPlayerManager::BroadcastTickInfo()
{
	std::vector<unsigned char> tickInfo = std::vector<unsigned char>();
	tickInfo.push_back(PacketHeader::TICK_INFO);
	tickInfo.push_back(m_ucPlayers);

	TickPacketFlags playerPacketFlags;

	for (unsigned char i = 0; i < m_ucMaxPlayers; ++i)
	{
		register CPlayer* pPlayer = m_ppPlayers[i];

		if (!pPlayer)
			continue;

		tickInfo.push_back(i);

		if (!pPlayer->IsSpawned())
		{
			playerPacketFlags.spawned = false;
			tickInfo.push_back(*(unsigned char*)&playerPacketFlags);
			continue;
		}

		playerPacketFlags.spawned = true;
		playerPacketFlags.inc_size = false;
		tickInfo.push_back(*(unsigned char*)&playerPacketFlags);

		tickInfo.push_back(pPlayer->GetHeadElement().x);
		tickInfo.push_back(pPlayer->GetHeadElement().y);
	}

	Broadcast(&tickInfo[0], tickInfo.size());
}

void CPlayerManager::Broadcast(const unsigned char* pBuf, const size_t sLen, const CPlayer* pPlayerToIgnore)
{
	for (unsigned char i = 0; i < m_ucMaxPlayers; ++i)
	{
		if (!m_ppPlayers[i] || m_ppPlayers[i]->GetAuthPhase() != AuthPhase::READY || m_ppPlayers[i] == pPlayerToIgnore)
			continue;

		m_ppPlayers[i]->SendPacket(pBuf, sLen);
	}
}

void CPlayerManager::BroadcastJoin(CPlayer* pPlayer)
{
	unsigned char* pBuf = new unsigned char[1+sizeof(unsigned char)+sizeof(unsigned char)+pPlayer->GetNickname().length()*sizeof(unsigned char)];
	pBuf[0] = PacketHeader::PLAYER_JOIN;
	pBuf[1] = GetPlayerId(pPlayer);
	pBuf[2] = pPlayer->GetNickname().length();
	memcpy(&pBuf[3], pPlayer->GetNickname().c_str(), pPlayer->GetNickname().length()*sizeof(unsigned char));

	Broadcast(pBuf, 1 + sizeof(unsigned char)+sizeof(unsigned char)+pPlayer->GetNickname().length()*sizeof(unsigned char), pPlayer);

	delete[] pBuf;
}

void CPlayerManager::BroadcastLeave(unsigned char ucId)
{
	unsigned char pBuf[2] = { PacketHeader::PLAYER_LEAVE, ucId };

	Broadcast(pBuf, 2, GetPlayerById(ucId));
}