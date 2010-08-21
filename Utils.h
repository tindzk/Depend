#import <Path.h>
#import <Logger.h>
#import <Terminal.h>
#import <FileStream.h>
#import <BufferedStream.h>

#define $(s) String(s)

extern Logger logger;

void Utils_OnLogMessage(__unused void *ptr, String msg, Logger_Level level, String file, int line);
bool File_IsModified(String sourceFile, String outputFile);
String File_GetContents(String filename);
String File_GetPath(String file);
String File_GetRealPath(String path);
