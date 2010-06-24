#include "Utils.h"

Logger logger;

extern ExceptionManager exc;

void Utils_OnLogMessage(UNUSED void *ptr, String msg, Logger_Level level, String file, int line) {
	String tmp;
	String slevel = Logger_LevelToString(level);

	if (BitMask_Has(logger.levels, Logger_Level_Debug)) {
		String sline = Integer_ToString(line);

		String_Print(tmp = String_Format(
			$("[%] % (%:%)\n"),
			slevel, msg, file, sline));
	} else {
		String_Print(tmp = String_Format(
			$("[%] %\n"), slevel, msg));
	}

	String_Destroy(&tmp);
}

String File_GetContents(String filename) {
	File file;
	BufferedStream stream;

	try (&exc) {
		FileStream_Open(&file, filename, O_RDONLY);
	} catch(&File_NotFoundException, e) {
		Logger_LogFmt(&logger, Logger_Level_Error,
			String("File '%' not found."), filename);

		exit(EXIT_FAILURE);
	} finally {

	} tryEnd;

	BufferedStream_Init(&stream, &FileStream_Methods, &file);
	BufferedStream_SetInputBuffer(&stream, 1024, 128);

	String s = HeapString(1024 * 15);

	size_t len = 0;

	do {
		len = File_Read(&file,
			s.buf  + s.len,
			s.size - s.len);

		s.len += len;
	} while (len > 0);

	BufferedStream_Close(&stream);
	BufferedStream_Destroy(&stream);

	return s;
}

bool File_IsModified(String sourceFile, String outputFile) {
	struct stat64 src = Path_GetStat(sourceFile);
	struct stat64 out = Path_GetStat(outputFile);

	return src.st_mtime > out.st_mtime;
}
