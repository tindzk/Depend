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

static def(bool, setOption, RdString name, RdString value) {
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
		Builder_map(&this->builder, value) || ret(false);
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
	} else if (String_Equals(name, $("library"))) {
		Builder_setLibrary(&this->builder,
			String_Equals(value, $("yes")));
	} else if (String_Equals(name, $("verbose"))) {
		Builder_setVerbose(&this->builder,
			String_Equals(value, $("yes")));
	} else {
		Logger_Error(this->logger, $("Unrecognized option '%'"), name);
	}

	return true;
}

static def(bool, readConfig, RdString path) {
	bool res = true;
	String contents = File_getContents(path);

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

		if (!call(setOption, name, value)) {
			res = false;
			break;
		}
	}

	String_Destroy(&contents);

	return res;
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

static def(void, build, RdString basePath) {
	Deps_scan(&this->deps, basePath);
	Builder_run(&this->builder);
}

static def(void, listDeps, RdString basePath) {
	Deps_scan(&this->deps, basePath);

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
		if (Path_exists(sys)) {
			call(readConfig, sys) || ret(false);
		}

		call(readConfig, args->buf[1]) || ret(false);

		String fullPath = String_New(0);
		RdString basePath = args->buf[1];

		if (Path_isFile(args->buf[1])) {
			fullPath = Path_expandFile(args->buf[1]);
			basePath = Path_getFolderPath(fullPath.rd);
		}

		if (action == ref(Action_Build)) {
			call(build, basePath);
		} else if (action == ref(Action_ListDeps)) {
			call(listDeps, basePath);
		}

		String_Destroy(&fullPath);
	}

	return true;
}
