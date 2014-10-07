#include "CPlayer.h"
#include "CServer.h"

CPlayer::CPlayer(libwebsocket* pLws, std::string strNickname)
{
	m_pLws = pLws;
	m_strName = strNickname;
	m_bSpawned = false;
	m_elements = std::vector<SnakeElement>();
	m_dir = SnakeDirection::NONE;
	m_authPhase = AuthPhase::AWAITING_INITIAL_INFO_RESPONSE;
}

void CPlayer::Spawn(unsigned char ucX, unsigned char ucY, unsigned int uiSize)
{
	unsigned char* pBuf = new unsigned char[2 + sizeof(unsigned int)+uiSize * 2];
	pBuf[0] = PacketHeader::PLAYER_SPAWN;
	pBuf[1] = CServer::GetPlayerManager()->GetPlayerId(this);
	*(unsigned int*)&pBuf[2] = uiSize;

	for (unsigned int i = 0; i < uiSize; ++i)
	{
		m_elements.push_back(SnakeElement{ ucX - i, ucY });
		pBuf[2 + sizeof(unsigned int)+2 * i] = ucX - i;
		pBuf[2 + sizeof(unsigned int)+2 * i + 1] = ucY;
	}

	m_dir = SnakeDirection::RIGHT;

	CServer::GetPlayerManager()->Broadcast(pBuf, 2 + sizeof(unsigned int)+uiSize * 2);

	m_bSpawned = true;
}

void CPlayer::Despawn()
{
	m_elements.clear();
	
	m_bSpawned = false;

	// TODO: send Despawn packet
}

void CPlayer::Step()
{
	for (unsigned int i = m_elements.size() - 1; i != 0; --i)
	{
		m_elements[i].x = m_elements[i - 1].x;
		m_elements[i].y = m_elements[i - 1].y;
	}

	switch (m_dir)
	{
	case SnakeDirection::UP:
		if (m_elements[0].y == 0)
			m_elements[0].y = 63;
		else
			--m_elements[0].y;

		break;

	case SnakeDirection::DOWN:
		++m_elements[0].y;
		break;

	case SnakeDirection::LEFT:
		if (m_elements[0].x == 0)
			m_elements[0].x = 63;
		else
			--m_elements[0].x;

		break;

	case SnakeDirection::RIGHT:
		++m_elements[0].x;
		break;

	}

	if (m_elements[0].x > 63)
		m_elements[0].x = 0;

	if (m_elements[0].y > 63)
		m_elements[0].y = 0;
}

int CPlayer::SendPacket(const unsigned char* pBuf, size_t sLen)
{
	return CServer::GetNetworkManager()->Send(m_pLws, pBuf, sLen);
}


SnakeDirection GetDirectionFromElements(SnakeElement el1, SnakeElement el2)
{
	if (el2.x > el1.x)
		return SnakeDirection::RIGHT;
	else if (el2.x < el1.x)
		return SnakeDirection::LEFT;
	else if (el2.y > el1.y)
		return SnakeDirection::DOWN;
	else if (el2.y < el1.y)
		return SnakeDirection::UP;

	return SnakeDirection::NONE;
}