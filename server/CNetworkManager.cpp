#include "CNetworkManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

fnCallback_t CNetworkManager::m_clientConnectCallback;
fnCallback_t CNetworkManager::m_clientDisconnectCallback;
fnCallback_t CNetworkManager::m_receiveCallback;

CNetworkManager::CNetworkManager(const int iPort, const unsigned int uiTimeout, fnCallback_t clientConnectCallback, fnCallback_t clientDisconnectCallback, fnCallback_t receiveCallback)
{
	struct lws_context_creation_info cci;
	memset(&cci, 0, sizeof(cci));

	m_ucLastError = 0;
	m_clientConnectCallback = clientConnectCallback;
	m_clientDisconnectCallback = clientDisconnectCallback;
	m_receiveCallback = receiveCallback;

	m_protocols[0].name = "snkmp";
	m_protocols[0].callback = WebSocketCallback;
	m_protocols[0].per_session_data_size = 0;
	m_protocols[0].rx_buffer_size = 65536;
	memset(&m_protocols[1], 0, sizeof(const char*)+sizeof(callback_function*)+sizeof(size_t));
	
	cci.port = iPort;
	cci.protocols = m_protocols;
	cci.extensions = libwebsocket_get_internal_extensions();
	cci.gid = -1;
	cci.uid = -1;

	if (!(m_pSockContext = libwebsocket_create_context(&cci)))
	{
		m_ucLastError = 1;
		return;
	}
}

CNetworkManager::~CNetworkManager()
{
	if (m_pSockContext)
		libwebsocket_context_destroy(m_pSockContext);

}


void CNetworkManager::Listen()
{
	while (true)
		libwebsocket_service(m_pSockContext, 50);

	delete this;
}

int CNetworkManager::WebSocketCallback(libwebsocket_context* ctx, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* data, size_t len)
{
	switch (reason)
	{
	case LWS_CALLBACK_ESTABLISHED:
		if (!m_clientConnectCallback(wsi, (unsigned char*)data, len))
			return -1;

		break;

	case LWS_CALLBACK_RECEIVE:
		if (!m_receiveCallback(wsi, (unsigned char*)data, len))
			return -1;

		break;

	case LWS_CALLBACK_CLOSED:
		if (!m_clientDisconnectCallback(wsi, (unsigned char*)data, len))
			return -1;

		break;
	}

	return 0;
}

int CNetworkManager::Send(libwebsocket* pLws, const unsigned char* pBuf, size_t sLen)
{
	unsigned char* pDataRaw = new unsigned char[LWS_SEND_BUFFER_PRE_PADDING + sLen + LWS_SEND_BUFFER_POST_PADDING];
	memcpy(pDataRaw + LWS_SEND_BUFFER_PRE_PADDING, pBuf, sizeof(unsigned char)*sLen);

	int iResult = libwebsocket_write(pLws, pDataRaw + LWS_SEND_BUFFER_PRE_PADDING, sLen, libwebsocket_write_protocol::LWS_WRITE_BINARY);

	delete[] pDataRaw;
	return iResult;
}

int CNetworkManager::SendError(libwebsocket* pLws, const char* pBuf, size_t sLen)
{
	unsigned char* pData = new unsigned char[sLen + 1];
	pData[0] = PacketHeader::CRITICAL_ERROR;
	memcpy(pData+1, pBuf, sLen);

	int iResult = Send(pLws, pData, sLen + 1);

	delete[] pData;
	return iResult;
}