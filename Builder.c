#import "Builder.h"

#define self Builder

rsdef(self, new, Terminal *term, Logger *logger, Deps *deps) {
	return (self) {
		.term      = term,
		.deps      = deps,
		.logger    = logger,
		.output    = String_Clone($("a.out")),
		.cc        = String_Clone($("/usr/bin/clang")),
		.inclhdr   = String_New(0),
		.manifest  = false,
		.dbgsym    = false,
		.std       = String_Clone($("gnu99")),
		.blocks    = true,
		.optmlevel = 0,
		.verbose   = false,
		.link      = StringArray_New(0),
		.mappings  = MappingArray_New(0),
		.linkpaths = StringArray_New(0),
		.workers   = CPU_getCores()
	};
}

def(void, destroy) {
	String_Destroy(&this->output);
	String_Destroy(&this->cc);
	String_Destroy(&this->inclhdr);
	String_Destroy(&this->std);

	StringArray_Destroy(this->link);
	StringArray_Free(this->link);

	StringArray_Destroy(this->linkpaths);
	StringArray_Free(this->linkpaths);

	each(item, this->mappings) {
		String_Destroy(&item->src);
		String_Destroy(&item->dest);
	}

	MappingArray_Free(this->mappings);
}

def(bool, map, RdString value) {
	bool src = true;
	RdString s = $("");
	DepsMapping insert;

	while (String_Split(value, ':', &s)) {
		if (src) {
			insert.src = String_Clone(s);
			src = false;
		} else {
			insert.dest = String_Clone(s);
			break;
		}
	}

	if (src) {
		Logger_Error(this->logger,
			$("`map' requires two values separated by a colon."));
		goto error;
	}

	if (insert.src.len == 0) {
		Logger_Error(this->logger, $("Invalid source path."));
		goto error;
	}

	if (!Path_Exists(insert.dest.rd)) {
		Logger_Error(this->logger,
			$("Destination path '%' does not exist."),
			insert.dest.rd);

		Terminal_Prompt prompt;
		Terminal_Prompt_Init(&prompt, this->term);

		bool isYes = Terminal_Prompt_Ask(&prompt, $$("Create path?"));

		Terminal_Prompt_Destroy(&prompt);

		if (isYes) {
			Path_Create(insert.dest.rd, true);
		} else {
			goto error;
		}
	}

	MappingArray_Push(&this->mappings, insert);

	return true;

error:
	String_Destroy(&insert.dest);
	String_Destroy(&insert.src);

	return false;
}

def(void, setOutput, RdString value) {
	String_Copy(&this->output, value);
}

def(void, setCompiler, RdString value) {
	String_Copy(&this->cc, value);
}

def(void, setInclHeader, RdString value) {
	String_Copy(&this->inclhdr, value);
}

def(void, setManifest, bool value) {
	this->manifest = value;
}

def(void, setDebuggingSymbols, bool value) {
	this->dbgsym = value;
}

def(void, setStandard, RdString value) {
	String_Copy(&this->std, value);
}

def(void, setBlocks, bool value) {
	this->blocks = value;
}

def(void, setOptimLevel, u16 value) {
	this->optmlevel = value;
}

def(void, setWorkers, u16 value) {
	this->workers = value;

	if (this->workers == 0) {
		/* Auto-detect workers. */
		this->workers = CPU_getCores();
	}
}

def(void, addLink, RdString value) {
	StringArray_Push(&this->link, String_Clone(value));
}

def(void, addLinkPath, RdString value) {
	StringArray_Push(&this->linkpaths, String_Clone(value));
}

def(void, setVerbose, bool value) {
	this->verbose = value;
}

static def(String, shrinkPath, RdString path) {
	each(mapping, this->mappings) {
		RdString shortPath = mapping->src.rd;
		String realPath    = Path_Resolve(shortPath);

		if (String_BeginsWith(path, realPath.rd)) {
			String res = String_New(0);

			if (!String_Equals(shortPath, '.')) {
				String_Append(&res, shortPath);
				String_Append(&res, '/');
			}

			String_Append(&res, String_Slice(path, realPath.len + 1));

			String_Destroy(&realPath);

			return res;
		}

		String_Destroy(&realPath);
	}

	return String_Clone(path);
}

def(void, onLinked, __unused pid_t pid, __unused int status) {
	EventLoop_Quit(EventLoop_GetInstance());
}

static def(void, link, StringArray *files) {
	Process proc = Process_new(this->cc.rd);

	Process_addParameter(&proc, $("-o"));
	Process_addParameter(&proc, this->output.rd);

	fwd(i, files->len) {
		Process_addParameter(&proc, files->buf[i].rd);
	}

	fwd(i, this->linkpaths->len) {
		Process_addParameter(&proc, $("-L"));
		Process_addParameter(&proc, this->linkpaths->buf[i].rd);
	}

	if (this->dbgsym) {
		Process_addParameter(&proc, $("-g"));
	}

	fwd(i, this->link->len) {
		if (this->link->buf[i].len == 0) {
			continue;
		}

		if (this->link->buf[i].buf[0] == '@') {
			Process_addParameter(&proc, $("-Wl,-Bdynamic"));
		} else {
			Process_addParameter(&proc, $("-Wl,-Bstatic"));
		}

		Process_addParameter(&proc, $("-l"));
		Process_addParameter(&proc, String_Slice(
			this->link->buf[i].rd,
			(this->link->buf[i].buf[0] == '@') ? 1 : 0));
	}

	pid_t pid = Process_spawn(&proc);
	Signal_uponChildTermination(Signal_GetInstance(), pid,
		Signal_OnChildTerminate_For(this, ref(onLinked)));

	if (this->verbose) {
		String cmd = Process_getCommandLine(&proc);
		Logger_Info(this->logger, cmd.rd);
		String_Destroy(&cmd);
	}

	Process_destroy(&proc);
}

static def(void, enqueue);

def(void, onCompiled, pid_t pid, int status) {
	Queue_setBuilt(&this->queue, pid);

	if (status == ExitStatus_Failure) {
		EventLoop_Quit(EventLoop_GetInstance());
		return;
	}

	if (Queue_hasNext(&this->queue)) {
		call(enqueue);
	} else if (Queue_getRunning(&this->queue) == 0) {
		StringArray *files = Queue_getLinkingFiles(&this->queue);

		call(link, files);

		StringArray_Destroy(files);
		StringArray_Free(files);

		EventLoop_Quit(EventLoop_GetInstance());
	}
}

static def(pid_t, compile, String src, String out) {
	Process proc = Process_new(this->cc.rd);

	Process_addParameter(&proc, $("-o"));
	Process_addParameter(&proc, out.rd);

	Process_addParameter(&proc, $("-c"));
	Process_addParameter(&proc, src.rd);

	String std = String_Concat($("-std="), this->std.rd);
	Process_addParameter(&proc, std.rd);
	String_Destroy(&std);

	if (this->blocks) {
		Process_addParameter(&proc, $("-fblocks"));
	}

	String strLevel = Integer_ToString(this->optmlevel);
	String optim    = String_Format($("-O%"), strLevel.rd);

	Process_addParameter(&proc, optim.rd);

	String_Destroy(&optim);
	String_Destroy(&strLevel);

	if (this->dbgsym) {
		Process_addParameter(&proc, $("-g"));
	}

	Process_addParameter(&proc, $("-W"));
	Process_addParameter(&proc, $("-Wall"));
	Process_addParameter(&proc, $("-Wextra"));
	Process_addParameter(&proc, $("-Wattributes"));
	Process_addParameter(&proc, $("-Wbad-function-cast"));
	Process_addParameter(&proc, $("-Wcast-align"));
	Process_addParameter(&proc, $("-Wcast-qual"));
	Process_addParameter(&proc, $("-Wchar-subscripts"));
	Process_addParameter(&proc, $("-Wdeclaration-after-statement"));
	Process_addParameter(&proc, $("-Wdisabled-optimization"));
	Process_addParameter(&proc, $("-Werror-implicit-function-declaration"));
	Process_addParameter(&proc, $("-Wfatal-errors"));
	Process_addParameter(&proc, $("-Wfloat-equal"));
	Process_addParameter(&proc, $("-Winit-self"));
	Process_addParameter(&proc, $("-Winline"));
	Process_addParameter(&proc, $("-Wmissing-declarations"));
	Process_addParameter(&proc, $("-Wmissing-field-initializers"));
	Process_addParameter(&proc, $("-Wmissing-format-attribute"));
	Process_addParameter(&proc, $("-Wmissing-include-dirs"));
	Process_addParameter(&proc, $("-Wmissing-noreturn"));
	Process_addParameter(&proc, $("-Wnested-externs"));
	Process_addParameter(&proc, $("-Wold-style-definition"));
	Process_addParameter(&proc, $("-Wpacked"));
	Process_addParameter(&proc, $("-Wparentheses"));
	Process_addParameter(&proc, $("-Wpointer-sign"));
	Process_addParameter(&proc, $("-Wredundant-decls"));
	Process_addParameter(&proc, $("-Wreturn-type"));
	Process_addParameter(&proc, $("-Wsequence-point"));
	Process_addParameter(&proc, $("-Wshadow"));
	Process_addParameter(&proc, $("-Wshorten-64-to-32"));
	Process_addParameter(&proc, $("-Wsign-compare"));
	Process_addParameter(&proc, $("-Wstrict-prototypes"));
	Process_addParameter(&proc, $("-Wswitch"));
	Process_addParameter(&proc, $("-Wswitch-default"));
	Process_addParameter(&proc, $("-Wswitch-enum"));
	Process_addParameter(&proc, $("-Wundef"));
	Process_addParameter(&proc, $("-Wuninitialized"));
	Process_addParameter(&proc, $("-Wunused"));
	Process_addParameter(&proc, $("-Wunused-parameter"));
	Process_addParameter(&proc, $("-Wunused-value"));
	Process_addParameter(&proc, $("-Wwrite-strings"));

	Process_addParameter(&proc, $("-fsigned-char"));

	Process_addParameter(&proc, $("-fstrict-aliasing"));
	Process_addParameter(&proc, $("-Wstrict-aliasing=2"));

	Process_addParameter(&proc, $("-fstrict-overflow"));
	Process_addParameter(&proc, $("-Wstrict-overflow=5"));

	Process_addParameter(&proc, $("-funit-at-a-time"));
	Process_addParameter(&proc, $("-fno-omit-frame-pointer"));

	Process_addParameter(&proc, $("-pipe"));

	if (this->inclhdr.len > 0) {
		Process_addParameter(&proc, $("-include"));
		Process_addParameter(&proc, this->inclhdr.rd);
	}

	StringArray *deps = Deps_getIncludes(this->deps);

	fwd(i, deps->len) {
		Process_addParameter(&proc, $("-I"));
		Process_addParameter(&proc, deps->buf[i].rd);
	}

	if (this->verbose) {
		String cmd = Process_getCommandLine(&proc);
		Logger_Info(this->logger, cmd.rd);
		String_Destroy(&cmd);
	}

	pid_t pid = Process_spawn(&proc);
	Signal_uponChildTermination(Signal_GetInstance(), pid,
		Signal_OnChildTerminate_For(this, ref(onCompiled)));

	Process_destroy(&proc);

	return pid;
}

static def(void, enqueue) {
	while (Queue_hasNext(&this->queue)
		&& Queue_getRunning(&this->queue) < this->workers)
	{
		Queue_Item *item = Queue_getNext(&this->queue);

		RdString create = Path_GetDirectory(item->output.rd);

		if (!Path_Exists(create)) {
			Path_Create(create, true);
		}

		String path = call(shrinkPath, item->source);

		String strCur   = Integer_ToString(Queue_getOffset(&this->queue));
		String strTotal = Integer_ToString(Queue_getTotal(&this->queue));

		Logger_Info(this->logger, $("Compiling %... [%/%]"),
			path.rd, strCur.rd, strTotal.rd);

		String_Destroy(&strCur);
		String_Destroy(&strTotal);

		pid_t pid = call(compile, path, item->output);
		Queue_setBuilding(&this->queue, item, pid);

		String_Destroy(&path);
	}
}

def(void, exit, __unused Signal_Type type) {
	Logger_Info(this->logger, $("Early exit."));
	EventLoop_Quit(EventLoop_GetInstance());
}

def(void, run) {
	if (this->manifest) {
		ManifestWriter manifest =
			ManifestWriter_new(this->logger, this->deps);
		ManifestWriter_create(&manifest);
		ManifestWriter_destroy(&manifest);
	}

	if (this->mappings->len == 0) {
		return;
	}

	this->queue = Queue_new(this->logger, this->deps, this->mappings);
	Queue_create(&this->queue);

	if (!Queue_hasNext(&this->queue)) {
		Logger_Info(this->logger, $("Nothing to do."));
		return;
	}

	String workers = Integer_ToString(this->workers);
	Logger_Info(this->logger, $("Using % worker(s)."), workers.rd);
	String_Destroy(&workers);

	Signal_listen(Signal_GetInstance());

	Signal_uponTermination(Signal_GetInstance(),
		Signal_OnTerminate_For(this, ref(exit)));

	call(enqueue);

	EventLoop_Run(EventLoop_GetInstance());

	Queue_destroy(&this->queue);
}
