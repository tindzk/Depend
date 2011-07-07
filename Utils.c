#import "Utils.h"

bool File_IsModified(RdString sourceFile, RdString outputFile) {
	Stat64 src = Path_getMeta(sourceFile);
	Stat64 out = Path_getMeta(outputFile);

	return src.mtime.sec > out.mtime.sec;
}
