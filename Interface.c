#import "Interface.h"

#define self Interface

def(void, init, Terminal *term, Logger *logger) {
	this->logger  = logger;
	this->deps    = Deps_new(logger);
	this->builder = Builder_new(term, logger, &this->deps);
}

def(void, destroy) {
	Builder_destroy(&this->builder);
	Deps_destroy(&this->deps);
}

static def(void, setOption, RdString name, RdString value) {
	if (String_Equals(name, $("debug"))) {
		if (String_Equals(value, $("yes"))) {
			BitMask_Set(this->logger->levels, Logger_Level_Debug);
		} else {
			BitMask_Clear(this->logger->levels, Logger_Level_Debug);
		}
	} else if (String_Equals(name, $("main"))) {
		Deps_setMain(&this->deps, value);
	} else if (String_Equals(name, $("add"))) {
		Deps_add(&this->deps, value);
	} else if (String_Equals(name, $("include"))) {
		Deps_addInclude(&this->deps, value);
	} else if (String_Equals(name, $("output"))) {
		Builder_setOutput(&this->builder, value);
	} else if (String_Equals(name, $("map"))) {
		Builder_map(&this->builder, value);
	} else if (String_Equals(name, $("cc"))) {
		Builder_setCompiler(&this->builder, value);
	} else if (String_Equals(name, $("inclhdr"))) {
		Builder_setInclHeader(&this->builder, value);
	} else if (String_Equals(name, $("manifest"))) {
		Builder_setManifest(&this->builder,
			String_Equals(value, $("yes")));
	} else if (String_Equals(name, $("dbgsym"))) {
		Builder_setDebuggingSymbols(&this->builder,
			String_Equals(value, $("yes")));
	} else if (String_Equals(name, $("std"))) {
		Builder_setStandard(&this->builder, value);
	} else if (String_Equals(name, $("blocks"))) {
		Builder_setBlocks(&this->builder,
			String_Equals(value, $("yes")));
	} else if (String_Equals(name, $("optimlevel"))) {
		Builder_setOptimLevel(&this->builder, UInt16_Parse(value));
	} else if (String_Equals(name, $("workers"))) {
		Builder_setWorkers(&this->builder, UInt16_Parse(value));
	} else if (String_Equals(name, $("link"))) {
		Builder_addLink(&this->builder, value);
	} else if (String_Equals(name, $("runtime"))) {
		Builder_setRuntime(&this->builder, value);
	} else if (String_Equals(name, $("linkpath"))) {
		Builder_addLinkPath(&this->builder, value);
	} else if (String_Equals(name, $("verbose"))) {
		Builder_setVerbose(&this->builder,
			String_Equals(value, $("yes")));
	} else {
		Logger_Error(this->logger, $("Unrecognized option '%'"), name);
	}
}

static def(void, readConfig, RdString path) {
	String contents = File_GetContents(path);

	RdString iter = $("");
	while (String_Split(contents.rd, '\n', &iter)) {
		RdString line = String_Trim(iter);

		if (line.len == 0 || line.buf[0] == '#') {
			continue;
		}

		RdString name, value;
		if (!String_Parse($("%=%"), line, &name, &value)) {
			Logger_Error(this->logger, $("Invalid line '%'"), line);
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

static def(void, build) {
	Deps_scan(&this->deps);
	Builder_run(&this->builder);
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

		/* System-wide settings. */
		RdString sys = $("/Settings/Depend.cfg");
		if (Path_Exists(sys)) {
			call(readConfig, sys);
		}

		call(readConfig, args->buf[1]);

		if (action == ref(Action_Build)) {
			call(build);
		} else if (action == ref(Action_ListDeps)) {
			call(listDeps);
		}
	}

	return true;
}
