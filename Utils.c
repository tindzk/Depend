#import "Utils.h"

extern Logger logger;
extern Terminal term;

void Utils_OnLogMessage(__unused void *ptr, FmtString msg, Logger_Level level, String file, int line) {
	String color  = $("black");
	String slevel = Logger_ResolveLevel(level);

	if (level == Logger_Level_Fatal || level == Logger_Level_Crit || level == Logger_Level_Error) {
		color = $("red");
	} else if (level == Logger_Level_Warn || level == Logger_Level_Info) {
		color = $("yellow");
	} else if (level == Logger_Level_Debug || level == Logger_Level_Trace) {
		color = $("cyan");
	}

	Terminal_Controller controller;
	Terminal_Controller_Init(&controller, &term);

	if (BitMask_Has(logger.levels, Logger_Level_Debug)) {
		String sline = Integer_ToString(line);

		Terminal_Controller_Render(&controller,
			$(".fg[%]{.b{[%]} $ .i{(%:%)}}\n"),
			color, slevel, msg, file, sline);
	} else {
		Terminal_Controller_Render(&controller,
			$(".fg[%]{.b{[%]} $}\n"),
			color, slevel, msg);
	}
}

bool File_IsModified(String sourceFile, String outputFile) {
	Stat64 src = Path_GetStat(sourceFile);
	Stat64 out = Path_GetStat(outputFile);

	return src.mtime.sec > out.mtime.sec;
}
