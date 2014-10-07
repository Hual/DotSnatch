#pragma once

#include <string>
#include <libwebsockets.h>
#include <vector>

enum SnakeDirection : unsigned char
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NONE
};

enum AuthPhase
{
	AWAITING_INITIAL_INFO_RESPONSE,
	READY
};

struct SnakeElement
{
	unsigned char x;
	unsigned char y;
};

struct TickPacketFlags
{
	unsigned char spawned : 1;
	unsigned char inc_size : 1;

};

class CPlayer
{
public:
	CPlayer(libwebsocket* pLws, std::string strNickname);

	void Spawn(unsigned char ucX, unsigned char ucY, unsigned int uiSize);
	void Despawn();
	void Step();
	int SendPacket(const unsigned char* pBuf, size_t sLen);

	inline std::string& GetNickname() { return m_strName; };
	inline libwebsocket* GetLws() { return m_pLws; };
	inline SnakeElement& GetElement(unsigned int uiIndex) { return m_elements[uiIndex]; };
	inline size_t GetElementCount() { return m_elements.size(); };
	inline SnakeElement& GetHeadElement() { return m_elements[0]; };
	inline std::vector<SnakeElement>& GetElements() { return m_elements; };
	inline bool IsSpawned() { return m_bSpawned; };
	inline SnakeDirection GetDirection() { return m_dir; };
	inline void SetDirection(SnakeDirection dir) { m_dir = dir; };
	inline AuthPhase GetAuthPhase() { return m_authPhase; };
	inline void SetAuthPhase(AuthPhase authPhase) { m_authPhase = authPhase; };

	static SnakeDirection GetDirectionFromElements(SnakeElement el1, SnakeElement el2);

private:
	libwebsocket* m_pLws;
	std::string m_strName;
	std::vector<SnakeElement> m_elements;
	SnakeDirection m_dir;
	AuthPhase m_authPhase;
	bool m_bSpawned;

};
