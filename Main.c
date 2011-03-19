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

	if (this->args->len == 0) {
		Logger_Error(&this->logger,
			$("Action missing. Run `% help' for an overview."),
			this->base);

		return false;
	}

	Interface itf;
	Interface_Init(&itf, &this->logger);
	Interface_SetAction(&itf, this->args->buf[0]);

	fwd(i, this->args->len) {
		RdString name, value;

		if (!String_Parse($("%=%"), this->args->buf[i], &name, &value)) {
			continue;
		}

		if (!Interface_SetOption(&itf, name, value)) {
			Interface_Destroy(&itf);
			return false;
		}
	}

	bool res = false;

	try {
		res = Interface_Run(&itf);
	} finally {
		Interface_Destroy(&itf);
	} tryEnd;

	return res;
}
