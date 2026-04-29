#ifndef JSONSTREAMSCANNER_H_INCLUDE
#define JSONSTREAMSCANNER_H_INCLUDE

#include <Arduino.h>

class JsonStreamScanner
{
public:
  JsonStreamScanner(Stream *stream, boolean chunked);
  String scanNextKey();
  String scanString();
  boolean scanBoolean();
  long scanInt();
  float scanFloat();

  String path();
  int available();

private:
  Stream *_stream;
  boolean _chunked;
  long _chunkSize;
  String _path;
  boolean _push;
  int _pathDepths[16];
  int _objDepth;
  int _peeked;
  boolean _hasPeeked;

  int readRawChar();
  int readChar();
  int peekChar();
  String readJsonString();
  void skipWhitespace();
  void skipPrimitive();
};

#endif
