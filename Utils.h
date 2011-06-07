#import <Path.h>
#import <Array.h>
#import <String.h>

record(DepsMapping) {
	String src;
	String dest;
};

Array(DepsMapping, MappingArray);

bool File_IsModified(RdString sourceFile, RdString outputFile);
