#include "JsonStreamScanner.h"

static long hexStrToLong(const String &s)
{
    return strtol(s.c_str(), NULL, 16);
}

JsonStreamScanner::JsonStreamScanner(Stream *stream, boolean chunked)
{
    _stream = stream;
    _chunked = chunked;
    _chunkSize = -1;
    _push = false;
    _objDepth = 0;
    _hasPeeked = false;
    _peeked = -1;
    memset(_pathDepths, 0, sizeof(_pathDepths));
}

// Read one character from the underlying stream, handling chunked transfer encoding
int JsonStreamScanner::readRawChar()
{
    unsigned long t = millis();
    while (!_stream->available())
    {
        if (millis() - t > 2000) return -1;
        delay(1);
    }
    if (!_chunked) return _stream->read();

    // Read chunk header when needed
    if (_chunkSize < 0)
    {
        String line = _stream->readStringUntil('\n');
        line.trim();
        // The CRLF terminating the previous chunk's data arrives as an empty line
        while (line.length() == 0)
        {
            unsigned long t2 = millis();
            while (!_stream->available())
            {
                if (millis() - t2 > 2000) return -1;
                delay(1);
            }
            line = _stream->readStringUntil('\n');
            line.trim();
        }
        _chunkSize = hexStrToLong(line);
        log_d("chunk: %ld", _chunkSize);
        if (_chunkSize == 0) return -1; // terminal chunk
    }

    int c = _stream->read();
    if (c != -1 && --_chunkSize == 0)
        _chunkSize = -1; // signal to read next chunk header
    return c;
}

int JsonStreamScanner::readChar()
{
    if (_hasPeeked)
    {
        _hasPeeked = false;
        return _peeked;
    }
    return readRawChar();
}

int JsonStreamScanner::peekChar()
{
    if (!_hasPeeked)
    {
        _peeked = readRawChar();
        _hasPeeked = true;
    }
    return _peeked;
}

void JsonStreamScanner::skipWhitespace()
{
    while (true)
    {
        int p = peekChar();
        if (p == ' ' || p == '\t' || p == '\r' || p == '\n')
            readChar();
        else
            break;
    }
}

// Read a JSON string from the stream (opening " already consumed).
// Handles all standard JSON escape sequences including \uXXXX (encoded as UTF-8).
String JsonStreamScanner::readJsonString()
{
    String result;
    while (true)
    {
        int c = readChar();
        if (c == -1 || c == '"') break;
        if (c != '\\')
        {
            result += (char)c;
            continue;
        }
        int e = readChar();
        switch (e)
        {
            case '"':  result += '"';  break;
            case '\\': result += '\\'; break;
            case '/':  result += '/';  break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            case 'b':  result += '\b'; break;
            case 'f':  result += '\f'; break;
            case 'u':
            {
                char hex[5] = {0};
                for (int i = 0; i < 4; i++)
                {
                    int h = readChar();
                    if (h == -1) break;
                    hex[i] = (char)h;
                }
                unsigned long cp = strtoul(hex, NULL, 16);
                if (cp < 0x80)
                {
                    result += (char)cp;
                }
                else if (cp < 0x800)
                {
                    result += (char)(0xC0 | (cp >> 6));
                    result += (char)(0x80 | (cp & 0x3F));
                }
                else
                {
                    result += (char)(0xE0 | (cp >> 12));
                    result += (char)(0x80 | ((cp >> 6) & 0x3F));
                    result += (char)(0x80 | (cp & 0x3F));
                }
                break;
            }
            default:
                result += (char)e;
                break;
        }
    }
    return result;
}

// Skip non-string value characters (numbers, true, false, null) up to a delimiter
void JsonStreamScanner::skipPrimitive()
{
    while (true)
    {
        int p = peekChar();
        if (p == -1 || p == ',' || p == '}' || p == ']' ||
            p == ' '  || p == '\t' || p == '\r' || p == '\n')
            break;
        readChar();
    }
}

// Scan the stream for the next JSON key and return its full path (e.g. "/item/name").
// Unmatched string values are skipped automatically. Object depth is tracked with a
// path-length stack so that closing braces always restore the correct prefix.
String JsonStreamScanner::scanNextKey()
{
    while (true)
    {
        skipWhitespace();
        int c = readChar();
        if (c == -1) break;

        switch (c)
        {
            case '{':
            {
                int idx = (_objDepth < 16) ? _objDepth : 15;
                _pathDepths[idx] = _path.length();
                _objDepth++;
                _push = true;
                break;
            }
            case '}':
            {
                if (_objDepth > 0)
                {
                    _objDepth--;
                    int idx = (_objDepth < 16) ? _objDepth : 15;
                    _path = _path.substring(0, _pathDepths[idx]);
                }
                _push = false;
                break;
            }
            case '[':
            case ']':
                // Arrays are transparent: they don't contribute path segments
                break;
            case ':':
            {
                // The previous key's value has not been consumed by the caller.
                // Skip primitive and string values; let '{' and '[' be handled naturally.
                skipWhitespace();
                int p = peekChar();
                if (p == '"')
                {
                    readChar(); // consume opening "
                    readJsonString();
                }
                else if (p != '{' && p != '[' && p != -1)
                {
                    skipPrimitive();
                }
                break;
            }
            case ',':
                _push = false;
                break;
            case '"':
            {
                String str = readJsonString();
                if (_push)
                {
                    _path += "/" + str;
                    _push = false;
                }
                else
                {
                    int lastSlash = _path.lastIndexOf('/');
                    if (lastSlash >= 0)
                        _path = _path.substring(0, lastSlash) + "/" + str;
                    else
                        _path = "/" + str;
                }
                return _path;
            }
            default:
                // Primitive value not preceded by ':' (e.g. bare array element)
                skipPrimitive();
                break;
        }
    }
    return "";
}

// Scan the string value for the most recently returned key.
// Stream must be positioned right after the key's closing ".
String JsonStreamScanner::scanString()
{
    int c;
    do { c = readChar(); } while (c != '"' && c != -1);
    if (c == -1) return "";
    return readJsonString();
}

// Scan a boolean value (true/false) for the most recently returned key.
boolean JsonStreamScanner::scanBoolean()
{
    int c;
    do { c = readChar(); } while (c != ':' && c != -1);
    skipWhitespace();
    c = readChar(); // 't' or 'f'
    boolean result = (c == 't');
    skipPrimitive();
    return result;
}

// Scan an integer value for the most recently returned key.
long JsonStreamScanner::scanInt()
{
    int c;
    do { c = readChar(); } while (c != ':' && c != -1);
    skipWhitespace();
    boolean negative = false;
    c = readChar();
    if (c == '-')
    {
        negative = true;
        c = readChar();
    }
    long result = 0;
    while (c >= '0' && c <= '9')
    {
        result = result * 10 + (c - '0');
        int p = peekChar();
        if (p < '0' || p > '9') break;
        c = readChar();
    }
    return negative ? -result : result;
}

// Scan a floating-point value for the most recently returned key.
float JsonStreamScanner::scanFloat()
{
    int c;
    do { c = readChar(); } while (c != ':' && c != -1);
    skipWhitespace();
    String numStr;
    if (peekChar() == '-') { numStr += '-'; readChar(); }
    while (true)
    {
        int p = peekChar();
        if ((p >= '0' && p <= '9') || p == '.' || p == 'e' || p == 'E' || p == '+' || p == '-')
            numStr += (char)readChar();
        else
            break;
    }
    return numStr.toFloat();
}

String JsonStreamScanner::path()
{
    return _path;
}

int JsonStreamScanner::available()
{
    return _hasPeeked ? 1 : _stream->available();
}
