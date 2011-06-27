#import <Main.h>
#import <Terminal/Controller.h>

#import "Utils.h"
#import "Interface.h"

#define self Application

def(void, OnLogMessage, FmtString msg, Logger_Level level, RdString file, int line) {
	RdString color  = $("black");
	RdString slevel = Logger_ResolveLevel(level);

	if (level == Logger_Level_Fatal || level == Logger_Level_Crit || level == Logger_Level_Error) {
		color = $("red");
	} else if (level == Logger_Level_Warn || level == Logger_Level_Info) {
		color = $("yellow");
	} else if (level == Logger_Level_Debug || level == Logger_Level_Trace) {
		color = $("cyan");
	}

	Terminal_Controller controller = Terminal_Controller_New(&this->term);

	if (BitMask_Has(this->logger.levels, Logger_Level_Debug)) {
		String sline = Integer_ToString(line);

		Terminal_Controller_Render(&controller,
			$(".fg[%]{.b{[%]} $ .i{(%:%)}}\n"),
			color, slevel, msg, file, sline.rd);

		String_Destroy(&sline);
	} else {
		Terminal_Controller_Render(&controller,
			$(".fg[%]{.b{[%]} $}\n"),
			color, slevel, msg);
	}
}

def(bool, Run) {
	Terminal_Configure(&this->term, true, true);
	BitMask_Clear(this->logger.levels, Logger_Level_Debug);

	Interface itf;
	Interface_Init(&itf, &this->term, &this->logger);

	bool res = false;

	try {
		res = Interface_run(&itf, this->args, this->base);
	} finally {
		Interface_Destroy(&itf);
	} tryEnd;

	return res;
}
