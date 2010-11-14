#import "Utils.h"
#import "Interface.h"

Logger logger;
Terminal term;
ExceptionManager exc;

int main(int argc, char* argv[]) {
	ExceptionManager_Init(&exc);

	File0(&exc);
	Path0(&exc);
	String0(&exc);

	Logger_Init(&logger, Callback(NULL, Utils_OnLogMessage),
		Logger_Level_Fatal |
		Logger_Level_Crit  |
		Logger_Level_Error |
		Logger_Level_Warn  |
		Logger_Level_Info  |
		Logger_Level_Trace);

	Terminal_Init(&term, File_StdIn, File_StdOut, true);
	Terminal_Configure(&term, true, true);

	if (argc <= 1) {
		Logger_Error(&logger,
			String("Action missing. Run `% help' for an overview."),
			String_FromNul(argv[0]));

		Terminal_Destroy(&term);

		return ExitStatus_Failure;
	}

	Interface itf;
	Interface_Init(&itf);
	Interface_SetAction(&itf, String_FromNul(argv[1]));

	bool success = true;

	for (int i = 1; i < argc; i++) {
		String arg = String_FromNul(argv[i]);

		ssize_t pos = String_Find(arg, '=');

		if (pos == String_NotFound) {
			continue;
		}

		String name  = String_Slice(arg, 0, pos);
		String value = String_Slice(arg, pos + 1);

		success = Interface_SetOption(&itf, name, value);

		if (!success) {
			goto out;
		}
	}

	try (&exc) {
		success = Interface_Run(&itf);
	} clean catchAny {
		ExceptionManager_Print(&exc, e);

#if Exception_SaveTrace
		Backtrace_PrintTrace(exc.e.trace, exc.e.traceItems);
#endif
	} finally {
		Terminal_Destroy(&term);
	} tryEnd;

out:
	Interface_Destroy(&itf);

	return success
		? ExitStatus_Success
		: ExitStatus_Failure;
}
