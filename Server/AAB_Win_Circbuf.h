#ifndef AAB_WIN_CIRCBUF_H
#define AAB_WIN_CIRCBUF_H

#include <Windows.h>
#include <stdlib.h>

#define CIRCBUF_OPT_NOWAIT			1
#define CIRCBUF_OPT_BLOCKTOCOMPLETE	2

// Circular Buffer Handle
#define CIRCBUF	struct AAB_Win_Circbuf*

int CircbufInit(CIRCBUF circbuf, int size);
int CircbufRead(CIRCBUF circbuf, void* readBuf, int toRead, int flags);
int CircbufWrite(CIRCBUF circbuf, void* writeBuf, int toWrite, int flags);
int CircBufSetBehavior(CIRCBUF circbuf, int readBehavior, int writeBehavior);

#endif