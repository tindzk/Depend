#import "Interface.h"

#define self Interface

def(void, Init, Terminal *term, Logger *logger) {
	this->logger = logger;
	this->deps = Deps_new(logger);

	Builder_Init(&this->builder, term, logger, &this->deps);
}

def(void, Destroy) {
	Builder_Destroy(&this->builder);
	Deps_destroy(&this->deps);
}

static def(bool, setOption, RdString name, RdString value) {
	if (String_Equals(name, $("debug"))) {
		if (String_Equals(value, $("yes"))) {
			BitMask_Set(this->logger->levels, Logger_Level_Debug);
		} else {
			BitMask_Clear(this->logger->levels, Logger_Level_Debug);
		}
	}

	if (!Deps_setOption(&this->deps, name, value)) {
		goto error;
	}

	if (!Builder_SetOption(&this->builder, name, value)) {
		goto error;
	}

	when (error) {
		Logger_Error(this->logger, $("Unrecognized option '%'"), name);
		return false;
	}

	return true;
}

static def(void, readConfig, RdString path) {
	String contents = File_GetContents(path);

	RdString line = $("");
	while (String_Split(contents.rd, '\n', &line)) {
		RdString name, value;
		if (!String_Parse($("%=%"), line, &name, &value)) {
			continue;
		}

		call(setOption, name, value);
	}

	String_Destroy(&contents);
}

static def(ref(Action), getAction, RdString action) {
	if (String_Equals(action, $("build"))) {
		return ref(Action_Build);
	} else if (String_Equals(action, $("deps"))) {
		return ref(Action_ListDeps);
	} else if (String_Equals(action, $("help"))) {
		return ref(Action_Help);
	}

	return ref(Action_Unknown);
}

static def(void, printHelp, RdString base) {
	Logger_Info(this->logger, $("Usage: % action [file]"), base);
	Logger_Info(this->logger,
		$("Supported actions are: build, deps, help."));
}

static def(bool, build) {
	Deps_scan(&this->deps);
	return Builder_Run(&this->builder);
}

static def(void, listDeps) {
	Deps_scan(&this->deps);

	Deps_Components *comps = Deps_getComponents(&this->deps);

	fwd(i, comps->len) {
		Logger_Info(this->logger, $("Source: %  Header: %"),
			comps->buf[i].source.rd, comps->buf[i].header.rd);
	}
}

def(bool, run, RdStringArray *args, RdString base) {
	if (args->len < 1) {
		Logger_Error(this->logger, $(
			"Action missing. "
			"Run `% help' for more information."), base);

		return false;
	}

	ref(Action) action = call(getAction, args->buf[0]);

	if (action == ref(Action_Unknown)) {
		Logger_Error(this->logger, $("Unknown action."));
		return false;
	}

	if (action == ref(Action_Help)) {
		call(printHelp, base);
	} else {
		if (args->len < 2) {
			Logger_Error(this->logger, $("Filename missing."));
			return false;
		}

		call(readConfig, args->buf[1]);

		if (action == ref(Action_Build)) {
			return call(build);
		} else if (action == ref(Action_ListDeps)) {
			call(listDeps);
		}
	}

	return true;
}
