#include "FileReader.h"
#include "saturnin/Assert.h"
#include <string.h>
#include <stdlib.h>

#define BLOCKSIZE ((unsigned int)4096)

using namespace saturnin;

FileReader::FileReader(FILE* f, unsigned int nbBlocks) : currentFile(f), bufSize(0), buf(nullptr), next(0), length(0), iseof(false) {
    bufSize = nbBlocks * BLOCKSIZE;
    buf = (char*) malloc(bufSize+1);
    ASSERT(buf != nullptr);
    buf[bufSize] = '\0';
}

FileReader::~FileReader() {
    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }
}

unsigned int FileReader::getNextLine(const char*& destination) {
    if (next >= length && !iseof) {
        length = static_cast<unsigned int>(fread(buf, sizeof (char), bufSize, currentFile));
        iseof = feof(currentFile) != 0;
        if (length < bufSize) {
            buf[length]='\0';
        }
        next = 0;
    }else if(next >= length && iseof){
        return 0;
    }
    return getNextLine(destination, next);
}

unsigned int FileReader::getNextLine(const char*& destination, unsigned int j) {

    while (j < length && buf[j] != '\n') {
        j++;
    }

    if ((j < length && buf[j] == '\n') || (j == length && iseof)) {
        //we found the position of the end of line
        //or we reached the end of the file
        destination = buf + next;
        j -= next;
        next += j + 1;
        return j;
    }

    //we don't have the whole line in the buffer

    if (next == 0) {
        //the buffer isn't big enough to hold the line
        unsigned int newSize = bufSize * 2;
        char* buf2 = (char*) malloc(newSize);
        ASSERT(buf2 != nullptr);
        memcpy(buf2, buf, bufSize);
        bufSize *= 2;
        //now that we have copied the data, we need to fill the rest of the
        //buffer
        unsigned int rd = (bufSize) - length;
        unsigned int bsize = BLOCKSIZE;
        if (rd % bsize != 0) {
            rd = bsize * (rd / bsize);
        }
        unsigned int l = static_cast<unsigned int>(fread(buf2 + length, sizeof (char), rd, currentFile));
        iseof = feof(currentFile) != 0;

        free(buf);

        buf = buf2;

        length += l;
        return getNextLine(destination, j);
    } else {
        ASSERT(length >= next);
        //the buffer might be big enough or we just reached the end of it
        memmove(buf, buf + next, bufSize - next);
        length -= next;

        //fill the space with new read characters
        unsigned int rd = (bufSize) - length;
        unsigned int bsize = BLOCKSIZE;
        if (rd % bsize != 0) {
            rd = bsize * (rd / bsize);
        }

        if (rd > 0) {
            unsigned int l = static_cast<unsigned int>(fread(buf + length, sizeof (char), rd, currentFile));
            iseof = feof(currentFile) != 0;
            length += l;

            //if read == 0, we will have to increase the capacity of the buffer.
            //it will be done at next recursion

        }

        j -= next;
        next = 0;
        return getNextLine(destination, j);
    }
}

bool FileReader::isEOF() {
    return next >= length && iseof;
}
