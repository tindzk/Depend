#import "Utils.h"
#import "Interface.h"

Logger logger;
Terminal term;

int main(int argc, char* argv[]) {
	Logger_Init(&logger, Callback(NULL, Utils_OnLogMessage),
		Logger_Level_Fatal |
		Logger_Level_Crit  |
		Logger_Level_Error |
		Logger_Level_Warn  |
		Logger_Level_Info  |
		Logger_Level_Trace);

	term = Terminal_New(File_StdIn, File_StdOut, true);
	Terminal_Configure(&term, true, true);

	if (argc <= 1) {
		Logger_Error(&logger,
			$("Action missing. Run `% help' for an overview."),
			String_FromNul(argv[0]));

		Terminal_Destroy(&term);

		return ExitStatus_Failure;
	}

	Interface itf;
	Interface_Init(&itf);
	Interface_SetAction(&itf, String_FromNul(argv[1]));

	bool success = true;

	for (int i = 1; i < argc; i++) {
		ProtString arg = String_FromNul(argv[i]);

		ssize_t pos = String_Find(arg, '=');

		if (pos == String_NotFound) {
			continue;
		}

		ProtString name  = String_Slice(arg, 0, pos);
		ProtString value = String_Slice(arg, pos + 1);

		success = Interface_SetOption(&itf, name, value);

		if (!success) {
			goto out;
		}
	}

	try {
		success = Interface_Run(&itf);
	} catchAny {
		Exception_Print(e);
	} finally {
		Terminal_Destroy(&term);
	} tryEnd;

out:
	Interface_Destroy(&itf);

	return success
		? ExitStatus_Success
		: ExitStatus_Failure;
}
