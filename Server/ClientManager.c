#include "Server.h"

DWORD WINAPI NewClientHandler(LPVOID params);

SOCKET clients[MAX_CLIENTS] = { 0 };

DWORD WINAPI ClientManager(LPVOID params){

	SOCKET listenSock;
	SOCKADDR_IN server = { 0 };
	fd_set set;
	HANDLE waits[2];
	int ret = 0;
	int i = 0;

	// Create the listening socket
	if ((listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, NULL, WSA_FLAG_OVERLAPPED)) == -1){
		perror("Listen socket");
		return -1;
	}

	// bind the server socket
	server.sin_family = AF_INET;
	server.sin_port = CONNECT_PORT;
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenSock, (SOCKADDR*)&server, sizeof(server)) == -1){
		perror("Listen bind");
		return -1;
	}

	// Queue up incoming clients
	listen(listenSock, 5);

	// launch the new client handler... I feel dirty just making this abomenation
	CreateThread(NULL, NULL, NewClientHandler, (LPVOID)listen, NULL, NULL);

	// set up the events
	waits[0] = newSongEvent;
	waits[1] = endOfProgramEvent;

	while (TRUE){

		ret = WaitForMultipleObjects(2, waits, FALSE, INFINITE);

		// error or end of program
		if (ret != WAIT_OBJECT_0){
			break;
		}

		// Update any valid clients with the new song info
		for (i = 0; i < MAX_CLIENTS; ++i){
			if (clients[i]){
				send(clients[i], &nowPlaying, sizeof(nowPlaying), 0);
			}
		}

	}



	return 0;
}

// The condition for accept that makes sure we have an available slot for the new socket
int CALLBACK ConditionFunc(LPWSABUF lpCallerId, LPWSABUF lpCallerData, LPQOS lpSQOS, LPQOS lpGQOS,
	LPWSABUF lpCalleeId, LPWSABUF lpCalleeData, GROUP FAR *g, DWORD_PTR dwCallbackData){

	int i = 0;
	for (i = 0; i < MAX_CLIENTS; ++i){
		if (!clients[i]){
			return CF_ACCEPT;
		}
	}
	return CF_REJECT;
}

// Handle new connections
DWORD WINAPI NewClientHandler(LPVOID params){
	SOCKADDR_IN newClientAddr = { 0 };
	int addrLen = 0;
	SOCKET newClient = 0;
	int i = 0;

	while (TRUE){

		// Accept a new condition if there's space
		newClient = WSAAccept((SOCKET)params, (SOCKADDR*)&newClientAddr, &addrLen, ConditionFunc, NULL);

		// ensure success
		if (newClient == SOCKET_ERROR){
			perror("Accept");
			return -1;
		}

		// find an available spot
		for (i = 0; i < MAX_CLIENTS; ++i){
			if (!clients[i]){
				break;
			}
		}
		if (i == MAX_CLIENTS){
			// should not happen, but best check
			close(newClient);
		}
		else{
			// add client and send info
			clients[i] = newClient;
			send(clients[i], &clientInfo, sizeof(clientInfo), 0);
			send(clients[i], &nowPlaying, sizeof(nowPlaying), 0);
		}
	}
}