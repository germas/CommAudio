#include <WinSock2.h>
#include <ws2tcpip.h>
#include "AAB_Win_Circbuf.h" // Which contains windows.h
#include <stdio.h>


#define OUTBUF_SIZE			2048
#define DATAGRAM_SIZE		1024
#define DATAGRAM_PER_SEC	30

#define MAX_NAME			128

HANDLE endOfProgramEvent;
HANDLE bufferingSem;

CIRCBUF outboundRadio;

DWORD WINAPI ClientManager(LPVOID params);
DWORD WINAPI RadioThread(LPVOID params);
DWORD WINAPI AudioBufferer(LPVOID params);

struct now_playing_data{
	char song[MAX_NAME];
	char artist[MAX_NAME];
	char album[MAX_NAME];
};

struct now_playing_data nowPlaying;