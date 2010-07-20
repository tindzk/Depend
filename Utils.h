#ifndef UTILS_H
#define UTILS_H

#include <Path.h>
#include <Logger.h>
#include <Terminal.h>
#include <FileStream.h>
#include <BufferedStream.h>

#define $(s) String(s)

extern Logger logger;

void Utils_OnLogMessage(UNUSED void *ptr, String msg, Logger_Level level, String file, int line);
bool File_IsModified(String sourceFile, String outputFile);
String File_GetContents(String filename);
String File_GetPath(String file);
String File_GetRealPath(String path);

#endif
