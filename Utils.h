#import <Path.h>
#import <Logger.h>
#import <Integer.h>
#import <Terminal.h>

#define $(s) String(s)

extern Logger logger;

void Utils_OnLogMessage(__unused void *ptr, String msg, Logger_Level level, String file, int line);
bool File_IsModified(String sourceFile, String outputFile);
