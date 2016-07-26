#include "Server.h"
#include <limits.h>

DWORD WINAPI AudioBufferer(LPVOID params){
	char song[MAX_PATH] = { 0 };
	char songPath[MAX_PATH] = { 0 };
	char infoPath[MAX_PATH] = { 0 };
	char path[MAX_PATH] = { 0 };
	FILE* audioFile;
	FILE* infoFile;

	char* title;
	char* artist;
	char* album;

	size_t len = 0;
	int read = 0;

	void* data = malloc(DATAGRAM_SIZE);

	HANDLE waits[2];

	int ret;
	

	memcpy(song, params, MAX_PATH);
	sprintf_s(path, MAX_PATH,  "..\\audio\\%s\\", song);
	sprintf_s(songPath, MAX_PATH, "%s%s.wav", path, song);
	sprintf_s(infoPath, MAX_PATH, "%sinfo.txt", path);

	// Open the audio
	fopen_s(&audioFile, songPath, "rb");
	if (!audioFile){
		printf("Could not find %s\n", song);
		return -1;
	}

	// open the info
	fopen_s(&infoFile, infoPath, "r");
	if (!infoFile){
		printf("Failed to find song info");
		return -1;
	}
	/*
	// get the title
	if (read = getline(&title, &len, infoFile) <= 0){
		printf("Could not read title");
		return -1;
	}
	if (len > MAX_NAME){
		printf("Warning: title too long.");
		len = MAX_NAME;
		title[MAX_NAME] = 0;
	}
	memset(nowPlaying.title, 0, MAX_NAME);
	memcpy(nowPlaying.title, title, len);

	// get the artist
	len = 0;
	if (read = getline(&artist, &len, infoFile) <= 0){
		printf("Could not read artist");
		return -1;
	}
	if (len > MAX_NAME){
		printf("Warning: artist too long.");
		len = MAX_NAME;
		title[MAX_NAME] = 0;
	}
	memset(nowPlaying.artist, 0, MAX_NAME);
	memcpy(nowPlaying.artist, artist, len);

	// get the album
	len = 0;
	if (read = getline(&album, &len, infoFile) <= 0){
		printf("Could not read album");
		return -1;
	}
	if (len > MAX_NAME){
		printf("Warning: album too long.");
		len = MAX_NAME;
		title[MAX_NAME] = 0;
	}
	memset(nowPlaying.album, 0, MAX_NAME);
	memcpy(nowPlaying.album, album, len);

	*/

	// Finished getting data
	// Wait for streaming slot

	waits[0] = bufferingSem;
	waits[1] = endOfProgramEvent;

	// Wait for our turn to buffer, or the end of the program
	ret = WaitForMultipleObjects(2, waits, FALSE, INFINITE);

	if (ret != WAIT_OBJECT_0){
		// Program terminated or wait abandoned
		return 0;
	}

	// Our turn to buffer

	printf("\n--------------------\n");
	//printf("Now playing: %s - %s [%s]\n", song, artist, album);
	printf("\n--------------------\n");

	SetEvent(newSongEvent);

	// Read a datagram of bytes from the file

	// ******************************************************
	// !!!!! REPLACE WITH BASE FUNCTION !!!!!!!
	// ******************************************************
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
	fclose(infoFile);
	/*free(title);
	free(artist);
	free(album);*/
	free(data);

	return 0;
}