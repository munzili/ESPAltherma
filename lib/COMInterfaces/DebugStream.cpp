#include "DebugStream.hpp"

#ifndef PIO_UNIT_TESTING

DebugStream::DebugStream(Stream* stream)
{
    this->stream = stream;
}

size_t DebugStream::printf(const char * format, ...)
{
    char loc_buf[64];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if(len < 0) {
        va_end(arg);
        return 0;
    }
    if(len >= (int)sizeof(loc_buf)){  // comparation of same sign type for the compiler
        temp = (char*) malloc(len+1);
        if(temp == NULL) {
            va_end(arg);
            return 0;
        }
        len = vsnprintf(temp, len+1, format, arg);
    }
    va_end(arg);
    len = stream->write((uint8_t*)temp, len);
    if(temp != loc_buf){
        free(temp);
    }
    return len;
}

inline size_t DebugStream::print(const __FlashStringHelper *ifsh)
{
    return stream->print(ifsh);
}

inline size_t DebugStream::print(const String& s)
{
    return stream->print(s);
}

inline size_t DebugStream::print(const char c[])
{
    return stream->print(c);
}

inline size_t DebugStream::print(char c)
{
    return stream->print(c);
}

inline size_t DebugStream::println(const __FlashStringHelper *ifsh)
{
    return stream->println(ifsh);
}

inline size_t DebugStream::println(const String& s)
{
    return stream->println(s);
}

inline size_t DebugStream::println(const char c[])
{
    return stream->println(c);
}

inline size_t DebugStream::println(char c)
{
    return stream->println(c);
}

inline size_t DebugStream::println(void)
{
    return stream->println();
}

#endif