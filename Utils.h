#import <Path.h>
#import <Logger.h>
#import <Terminal.h>
#import <FileStream.h>
#import <BufferedStream.h>

#define $(s) String(s)

extern Logger logger;

void Utils_OnLogMessage(__unused void *ptr, String msg, Logger_Level level, String file, int line);
String File_GetContents(String filename);
bool File_IsModified(String sourceFile, String outputFile);
