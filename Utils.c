#import "Utils.h"

Logger logger;

extern Terminal term;
extern ExceptionManager exc;

void Utils_OnLogMessage(__unused void *ptr, String msg, Logger_Level level, String file, int line) {
	String fmt;
	String slevel = Logger_LevelToString(level);

	if (BitMask_Has(logger.levels, Logger_Level_Debug)) {
		String sline = Integer_ToString(line);

		fmt = String_Format($("[%] % (%:%)\n"),
			slevel, msg, file, sline);
	} else {
		fmt = String_Format($("[%] %\n"), slevel, msg);
	}

	if (level == Logger_Level_Error) {
		Terminal_Print(&term,
			Terminal_Color_ForegroundRed,
			Terminal_Font_Normal,
			fmt);
	} else if (level == Logger_Level_Info) {
		Terminal_Print(&term,
			Terminal_Color_ForegroundCyan,
			Terminal_Font_Normal,
			fmt);
	} else if (level == Logger_Level_Debug) {
		Terminal_Print(&term,
			Terminal_Color_ForegroundYellow,
			Terminal_Font_Normal,
			fmt);
	} else {
		Terminal_Print(&term,
			Terminal_Color_Normal,
			Terminal_Font_Normal,
			fmt);
	}

	String_Destroy(&fmt);
}

String File_GetContents(String filename) {
	File file;
	BufferedStream stream;

	try (&exc) {
		FileStream_Open(&file, filename, FileStatus_ReadOnly);
	} catch(File_NotFoundException, e) {
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
	Stat64 src = Path_GetStat(sourceFile);
	Stat64 out = Path_GetStat(outputFile);

	return src.mtime.sec > out.mtime.sec;
}
