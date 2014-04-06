#include "Server.h"

DWORD WINAPI AudioTimer(LPVOID params);
HANDLE timerTick;
BOOL running = TRUE;

char multicastAddr[] = "234.5.6.7";
int targetPort = 43392;

SOCKET radioOutSock = 0;				// The socket for multicasting
SOCKADDR_IN multicastOut = { 0 };		// The multicast address

DWORD WINAPI RadioThread(LPVOID params){

	SOCKADDR_IN serverAddr = { 0 };			// The local address
	struct ip_mreq multicastGroup = { 0 };	// The multicast session info

	void* outData = malloc(DATAGRAM_SIZE);
	WSAOVERLAPPED outOvr = { 0 };
	outOvr.hEvent = WSACreateEvent();
	DWORD sent = 0;
	DWORD flags = 0;

	BOOL disable = FALSE;
	BOOL enable = TRUE;

	HANDLE waits[2];
	int ret;

	// Create the socket to multicast on
	if ((radioOutSock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, NULL, WSA_FLAG_OVERLAPPED)) == -1){
		printf("Failed to create radio socket\n");
		printf("Err: %d\n", WSAGetLastError());
		return -1;
	}

	// Set up the local end
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = 0; // any port
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // any address

	// Bind the socket on the local end
	// Not required outside microsoft
	if (bind(radioOutSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == -1){
		printf("Failed to bind radio socket\n");
		printf("Err: %d\n", WSAGetLastError());
		return -1;
	}

	// Set up the multicast session
	multicastGroup.imr_interface.s_addr = htonl(INADDR_ANY); // Any local interface
	multicastGroup.imr_multiaddr.s_addr = inet_addr(multicastAddr); // The multicast address

	// Add the address info to the client info struct
	clientInfo.radio_addr = inet_addr(multicastAddr);
	clientInfo.mic_addr = inet_addr("234.5.6.8");

	// Join our own multicast session
	// Not required outside windows
	if (setsockopt(radioOutSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&multicastGroup, sizeof(multicastGroup)) == -1){
		printf("Failed to join own multicast session\n");
		printf("Err: %d\n", WSAGetLastError());
		return -1;
	}

	// Disable loopback
	if (setsockopt(radioOutSock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&disable, sizeof(disable)) == -1){
		printf("Failed to disable loopback\n");
		printf("Err: %d\n", WSAGetLastError());
		return -1;
	}


	// Set up the outbound address to be the multicast
	multicastOut.sin_family = AF_INET;
	multicastOut.sin_port = htons(targetPort);	// The port sending TO
	multicastOut.sin_addr.s_addr = inet_addr(multicastAddr);
	
	// Start timer
	running = 1;
	timerTick = CreateEvent(NULL, FALSE, FALSE, NULL);
	CreateThread(NULL, NULL, AudioTimer, NULL, NULL, NULL);

	// Sending loop
	waits[0] = timerTick;
	waits[1] = endOfProgramEvent;
	
	while (TRUE){
		ret = WaitForMultipleObjects(2, waits, FALSE, INFINITE);

		if (ret != WAIT_OBJECT_0){
			// program killed
			running = 0;
			break;
		}

		// Read the latest audio data
		CircbufRead(outboundRadio, outData, DATAGRAM_SIZE, CIRCBUF_OPT_BLOCKTOCOMPLETE);

		// Multicast out
		sendto(radioOutSock, outData, DATAGRAM_SIZE, 0, (SOCKADDR*)&multicastOut, sizeof(multicastOut));

	}

	// cleanup
	free(outData);

	return 0;
}

DWORD WINAPI AudioTimer(LPVOID params){
	int sleepTime = 1000 / DATAGRAM_PER_SEC;

	while (running){
		Sleep(sleepTime);
		SetEvent(timerTick);
	}

	return 0;
}