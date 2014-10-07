STD=c++11
CC=g++
CFLAGS=-c -Wall -Wno-switch -O2 -fpermissive -Wno-error -Wno-unknown-pragmas -Wno-invalid-offsetof -I./libwebsockets/include
BFLAGS= -Wl,--whole-archive libwebsockets/lib/libwebsockets.a -Wl,--no-whole-archive -lpthread -lz -lssl -lcrypto -lrt
SOURCES=CNetworkManager.cpp CPlayer.cpp CPlayerManager.cpp CServer.cpp main.cpp NetworkCallback.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=bin/server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) -std=$(STD) $(BFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -std=$(STD) $< -o $@ -lssl -lcrypto -lwebsockets -lrt

clean:
	rm -rf *.o