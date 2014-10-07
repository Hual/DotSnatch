#include <stdlib.h>
#include <stdio.h>
#include "CServer.h"
//#include <libwebsockets.h>

//static unsigned char msg[] = "welcum m80";

/*static int echoCallback(struct libwebsocket_context* ctx, struct libwebsocket* wsi, enum libwebsocket_callback_reasons reason, void* user, void* data, size_t len)
{
	switch (reason)
	{
	case LWS_CALLBACK_ESTABLISHED:
		libwebsocket_write(wsi, msg, sizeof(msg), LWS_WRITE_TEXT);
		printf("client connected: 0x%X\n", wsi);
		break;
	
	case LWS_CALLBACK_RECEIVE:
		printf("received data from user 0x%X: %s\n (size=%u)\n", wsi, (char*)data, len);
		return -1;
		break;

	case LWS_CALLBACK_CLOSED:
		printf("client closed connection: 0x%X!\n", wsi);
		break;

	}

	return 0;
}

static struct libwebsocket_protocols protocols[] = {
	{
		"echo",
		echoCallback,
		0
	},
	{
		NULL, NULL, 0
	}
};
*/
int main()
{
	/*struct libwebsocket_context* context;
	struct lws_context_creation_info cci;
	memset(&cci, 0, sizeof(cci));

	lws_set_log_level(0, lwsl_emit_syslog);

	cci.port = /*25123*/
	
	CServer::Initialize(7777, 7);
	
	/*7777;
	cci.iface = NULL;
	cci.protocols = protocols;
	cci.extensions = libwebsocket_get_internal_extensions();
	cci.ssl_cert_filepath = NULL;
	cci.ssl_private_key_filepath = NULL;
	cci.gid = -1;
	cci.uid = -1;
	cci.options = 0;

	if (!(context = libwebsocket_create_context(&cci)))
	{
		fprintf(stderr, "web socket context creation failed\n");
		return -1;
	}

	printf("starting server...\n");

	while (true)
	{
		libwebsocket_service(context, 10);
	}

	libwebsocket_context_destroy(context);*/
	return 0;
}

