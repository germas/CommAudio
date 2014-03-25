#include "AAB_Win_Circbuf.h"

// A circular buffer implementation
struct AAB_Win_Circbuf{
	void* _o_Offset;	// origin of the buffer
	void* _x_Offset;	// end of the buffer
	void* _r_Offset;	// current read offset
	void* _w_Offset;	// current write offset
	HANDLE _wSem;		// writing semaphore
	HANDLE _rSem;		// reading semaphore
	HANDLE _aSem;		// semaphore for all internal data operations
	int _size;			// size of the buffer
	int _contentSize;	// amount of data currently in the buffer
};

int _AAB_Win_Circbuf_defaultReadBehavior = CIRCBUF_OPT_NOWAIT;
int _AAB_Win_Circbuf_defaultWriteBehavior = CIRCBUF_OPT_NOWAIT;

/*
	Creates a Circular buffer of size
	Returns the size of the buffer, or -1 on failure
*/
int CircbufInit(CIRCBUF* circbuf, int size){
	*circbuf = malloc(sizeof(struct AAB_Win_Circbuf));
	if (!*circbuf){
		return -1;
	}
	(*circbuf)->_o_Offset = malloc(size);
	if (!(*circbuf)->_o_Offset){
		return -1;
	}
	(*circbuf)->_x_Offset = (long)(*circbuf)->_o_Offset + size;
	(*circbuf)->_w_Offset = (*circbuf)->_o_Offset;
	(*circbuf)->_r_Offset = (*circbuf)->_o_Offset;
	(*circbuf)->_size = size;
	(*circbuf)->_contentSize = 0;
	(*circbuf)->_wSem = CreateSemaphore(NULL, 1l, 1l, NULL);
	(*circbuf)->_rSem = CreateSemaphore(NULL, 0l, 1l, NULL);
	(*circbuf)->_aSem = CreateSemaphore(NULL, 1l, 1l, NULL);
	if (!(*circbuf)->_wSem || !(*circbuf)->_rSem || !(*circbuf)->_aSem){
		return -1;
	}
	return size;
}

/*
	Read From the buffer
	Returns the number of bytes read
*/
int CircbufRead(CIRCBUF circbuf, void* readBuf, int toRead, int flags){
	int i = 0;
	int ret = 0;
	int wait = 0;
	int read = 0;
	int moreToRead = 1;
	int prevContentSize;

	// Establish the flags
	if (!flags){
		flags = _AAB_Win_Circbuf_defaultReadBehavior;
	}
	if (flags == CIRCBUF_OPT_NOWAIT){
		wait = 0;
	}
	if (flags == CIRCBUF_OPT_BLOCKTOCOMPLETE){
		wait = INFINITE;
	}
	
	// While we want to keep reading from the buffer
	while (moreToRead){
		// Get access to the read buffer
		// If the buffer is empty, this will block until a write adds something to it
		ret = WaitForSingleObject(circbuf->_rSem, wait);
		// If we got access
		if (ret == WAIT_OBJECT_0){
			// track the previous content size
			prevContentSize = circbuf->_contentSize;

			// While there's more to read, and the buffer is not empty
			for (i = i; i < toRead && circbuf->_contentSize > 0; ++i){
				// Read a byte
				memcpy(readBuf, circbuf->_r_Offset, 1);
				// Increment the read count, and move the pointers
				read++;
				// Wait for the data lock to make operations atomic
				WaitForSingleObject(circbuf->_aSem, INFINITE);
				circbuf->_contentSize--;
				((long)readBuf)++;
				((long)circbuf->_r_Offset)++;
				// Check to see if we're at the loop point
				if (circbuf->_r_Offset > circbuf->_x_Offset){
					circbuf->_r_Offset = circbuf->_o_Offset;
				}
				ReleaseSemaphore(circbuf->_aSem, 1l, NULL);
				
			}
			// If we read all that we wanted to, or as much as we could with nowait, exit
			if (i == toRead || flags == CIRCBUF_OPT_NOWAIT){
				moreToRead = 0;
			}
		}
		// if we timed out, just exit
		else if (ret == WAIT_TIMEOUT){
			return read;
		}
	}
	// We read all we wanted to and the buffer is not empty

	// check to see if a write could be waiting on space
	if (prevContentSize == circbuf->_size && circbuf->_contentSize < circbuf->_size){
		ReleaseSemaphore(circbuf->_wSem, 1l, NULL);
	}
	ReleaseSemaphore(circbuf->_rSem, 1l, NULL);
	return read;
	
}

/*
	Write To the buffer
	Returns the number of bytes written
*/
int CircbufWrite(CIRCBUF circbuf, void* writeBuf, int toWrite, int flags){
	int i = 0;
	int ret = 0;
	int wait = 0;
	int written = 0;
	int moreToWrite = 1;
	int prevContentSize = 0;

	// Establish the flags
	if (!flags){
		flags = _AAB_Win_Circbuf_defaultWriteBehavior;
	}
	if (flags == CIRCBUF_OPT_NOWAIT){
		wait = 0;
	}
	if (flags == CIRCBUF_OPT_BLOCKTOCOMPLETE){
		wait = INFINITE;
	}

	// While we want to keep writing to the buffer
	while (moreToWrite){
		// Get access to the write buffer
		// if the buffer is full, this will block waiting on space
		ret = WaitForSingleObject(circbuf->_wSem, wait);
		// If we got access
		if (ret == WAIT_OBJECT_0){
			// Store the previous content size
			prevContentSize = circbuf->_contentSize;

			// While there's more to write, and the buffer is not full
			for (i = i; i < toWrite && circbuf->_contentSize < circbuf->_size; ++i){
				// Write a byte
				memcpy(circbuf->_w_Offset, writeBuf, 1);
				// Increment the written count, and move the pointers
				written++;
				// Wait for the data lock to make operations atomic
				WaitForSingleObject(circbuf->_aSem, INFINITE);
				circbuf->_contentSize++;
				((long)writeBuf)++;
				((long)circbuf->_w_Offset)++;
				// Check to see if we're at the loop point
				if (circbuf->_w_Offset > circbuf->_x_Offset){
					circbuf->_w_Offset = circbuf->_o_Offset;
				}
				ReleaseSemaphore(circbuf->_aSem, 1l, NULL);

			}
			// If we read all that we wanted to, or as much as we could with nowait, exit
			if (i == toWrite || flags == CIRCBUF_OPT_NOWAIT){
				moreToWrite = 0;
			}
		}
		// if we timed out, just exit
		else if (ret == WAIT_TIMEOUT){
			return written;
		}
	}
	// We read all we wanted to.
	ReleaseSemaphore(circbuf->_wSem, 1l, NULL);
	// If the content was empty before the write, and isn't now, signal the read sem
	// so a thread waiting to read will be able to
	if (prevContentSize == 0 && circbuf->_contentSize > 0){
		ReleaseSemaphore(circbuf->_rSem, 1l, NULL);
	}
	return written;
}

/*
	Sets the default behavior of the buffer
	retruns the number of valid options set
*/
int CircBufSetBehavior(CIRCBUF circbuf, int readBehavior, int writeBehavior){
	int success = 0;
	if (readBehavior == CIRCBUF_OPT_NOWAIT || readBehavior == CIRCBUF_OPT_BLOCKTOCOMPLETE){
		_AAB_Win_Circbuf_defaultReadBehavior = readBehavior;
		success++;
	}
	if (writeBehavior == CIRCBUF_OPT_NOWAIT || writeBehavior == CIRCBUF_OPT_BLOCKTOCOMPLETE){
		_AAB_Win_Circbuf_defaultWriteBehavior = writeBehavior;
		success++;
	}
	return success;
}