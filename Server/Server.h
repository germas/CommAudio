#define _POSIX_C_SOURCE

#include <WinSock2.h>
#include <ws2tcpip.h>
#include "AAB_Win_Circbuf.h" // Which contains windows.h
#include <stdio.h>


#define OUTBUF_SIZE			2048
#define DATAGRAM_SIZE		1024
#define DATAGRAM_PER_SEC	30

#define MAX_NAME			128
#define MAX_CLIENTS			64

#define CONNECT_PORT		55324

HANDLE endOfProgramEvent;
HANDLE newSongEvent;

HANDLE bufferingSem;

CIRCBUF outboundRadio;

DWORD WINAPI ClientManager(LPVOID params);
DWORD WINAPI RadioThread(LPVOID params);
DWORD WINAPI AudioBufferer(LPVOID params);

struct now_playing_data{
	char title[MAX_NAME];
	char artist[MAX_NAME];
	char album[MAX_NAME];
};

struct client_info{
	long radio_addr;
	long mic_addr;
};

struct now_playing_data nowPlaying;
struct client_info clientInfo;