#pragma once

#include <libwebsockets.h>

#define WEBSOCK_CALLBACK_RETURN bool
#define WEBSOCK_CALLBACK_ARGS libwebsocket *pId, unsigned char* pData, size_t sLen

typedef WEBSOCK_CALLBACK_RETURN (*fnCallback_t)(WEBSOCK_CALLBACK_ARGS);

enum PacketHeader : unsigned char
{
	AUTH_REQUEST,
	AUTH_RESPONSE,
	CRITICAL_ERROR,
	INITIAL_INFO_REQUEST,
	INITIAL_INFO_RESPONSE,
	TICK_INFO,
	DIRECTION_CHANGE,
	PLAYER_JOIN,
	PLAYER_SPAWN,
	PLAYER_LEAVE
};

class CNetworkManager
{
public:
	CNetworkManager(const int iPort, const unsigned int uiTimeout, fnCallback_t clientConnectCallback, fnCallback_t clientDisconnectCallback, fnCallback_t receiveCallback);
	virtual ~CNetworkManager();
	void Listen();
	int Send(libwebsocket* pLws, const unsigned char* pBuf, size_t sLen);
	int SendError(libwebsocket* pLws, const char* pBuf, size_t sLen);

private:
	static int WebSocketCallback(struct libwebsocket_context* ctx, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void* user, void* data, size_t len);

	libwebsocket_context* m_pSockContext;
	libwebsocket_protocols m_protocols[2];
	unsigned char m_ucLastError;
	static fnCallback_t m_clientConnectCallback;
	static fnCallback_t m_clientDisconnectCallback;
	static fnCallback_t m_receiveCallback;

};