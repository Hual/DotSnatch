#pragma once

#include "CNetworkManager.h"
#include "CServer.h"

#define RETURN_ERROR(lws, error, len) { CServer::GetNetworkManager()->SendError(lws, error, len); return false; }

class NetworkCallback
{
public:
	static WEBSOCK_CALLBACK_RETURN ClientConnectCallback(WEBSOCK_CALLBACK_ARGS);
	static WEBSOCK_CALLBACK_RETURN ClientDisconnectCallback(WEBSOCK_CALLBACK_ARGS);
	static WEBSOCK_CALLBACK_RETURN ReceiveCallback(WEBSOCK_CALLBACK_ARGS);

};