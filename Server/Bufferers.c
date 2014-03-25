#include "Server.h"
#include <limits.h>

DWORD WINAPI AudioBufferer(LPVOID params){
	char song[MAX_PATH] = { 0 };
	char path[MAX_PATH] = { 0 };
	FILE* audioFile;

	void* data = malloc(DATAGRAM_SIZE);

	HANDLE waits[2];

	int ret;
	

	memcpy(song, params, MAX_PATH);
	sprintf_s(path, MAX_PATH,  "..\\audio\\%s\\%s.wav", song, song);

	fopen_s(&audioFile, path, "rb");

	if (!audioFile){
		printf("Could not find %s\n", song);
		return -1;
	}

	waits[0] = bufferingSem;
	waits[1] = endOfProgramEvent;

	// Wait for our turn to buffer, or the end of the program
	ret = WaitForMultipleObjects(2, waits, FALSE, INFINITE);

	if (ret != WAIT_OBJECT_0){
		// Program terminated or wait abandoned
		return 0;
	}

	// Our turn to buffer

	printf("\nNow playing: %s\n", song);

	// Read a datagram of bytes from the file
	while ((ret = fread(data, 1, DATAGRAM_SIZE, audioFile)) == DATAGRAM_SIZE){
		// Not eof
		// Write data to buffer
		CircbufWrite(outboundRadio, data, DATAGRAM_SIZE, CIRCBUF_OPT_BLOCKTOCOMPLETE);
	}

	// eof, write whatever was left
	CircbufWrite(outboundRadio, data, ret, CIRCBUF_OPT_BLOCKTOCOMPLETE);

	// Done, allow next bufferer in
	ReleaseSemaphore(bufferingSem, 1l, NULL);

	// Cleanup
	fclose(audioFile);
	free(data);

	return 0;
}