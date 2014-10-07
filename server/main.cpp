#include <stdlib.h>
#include <stdio.h>
#include "CServer.h"

int main()
{
	CServer::Initialize(7777, 7);
	return 0;
}

