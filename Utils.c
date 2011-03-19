#import "Utils.h"

bool File_IsModified(RdString sourceFile, RdString outputFile) {
	Stat64 src = Path_GetStat(sourceFile);
	Stat64 out = Path_GetStat(outputFile);

	return src.mtime.sec > out.mtime.sec;
}
