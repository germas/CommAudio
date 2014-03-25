#include "Server.h"

int main(int argc, char* argv[]){

	DWORD thread = 0;
	WSADATA wsaDat;
	outboundRadio = malloc(sizeof(CIRCBUF));

	CircbufInit(&outboundRadio, 2 * DATAGRAM_SIZE);
	endOfProgramEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	bufferingSem = CreateSemaphore(NULL, 1l, 1l, NULL);
	char addFile[MAX_PATH];

	memset(&nowPlaying, 0, sizeof(nowPlaying));

	if (WSAStartup(0x0202, &wsaDat)){
		printf("Failed to startup.\n");
		return -1;
	}

	// Launch the radio sending thread
	CreateThread(NULL, NULL, RadioThread, NULL, NULL, &thread);

	// Prompt for new files to buffer
	do{
		printf("Enter a song to queue or q to quit: ");
		fgets(addFile, sizeof(addFile), stdin);
		addFile[strlen(addFile) - 1] = 0;
		if (strcmp(addFile, "q")){
			addBufferer(addFile);
		}
		Sleep(500);
	} while (strcmp(addFile, "q"));

	// Signal the end of the program
	SetEvent(endOfProgramEvent);
	
	// Wait for the radio thread to exit
	WaitForSingleObject(thread, INFINITE);
	
	printf("Terminated\n");
	getchar();

	return 0;
}

DWORD addBufferer(char* file){
	DWORD id = 0;
	CreateThread(NULL, NULL, AudioBufferer, file, NULL, &id);
	return id;
}