#import "Builder.h"

#define self Builder

rsdef(self, new, Terminal *term, Logger *logger, Deps *deps) {
	return (self) {
		.term      = term,
		.deps      = deps,
		.logger    = logger,
		.output    = String_New(0),
		.cc        = String_Clone($("/usr/bin/clang")),
		.inclhdr   = String_New(0),
		.runtime   = String_New(0),
		.manifest  = false,
		.library   = false,
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
	String_Destroy(&this->runtime);
	String_Destroy(&this->std);

	StringArray_Destroy(this->link);
	StringArray_Free(this->link);

	StringArray_Destroy(this->linkpaths);
	StringArray_Free(this->linkpaths);

	each(item, this->mappings) {
		String_Destroy(&item->src);
		String_Destroy(&item->dest);
		String_Destroy(&item->namespace);
	}

	MappingArray_Free(this->mappings);
}

def(bool, map, RdString value) {
	RdString src, dest, namespace;

	if (!String_Parse($("%:%:%"), value, &src, &dest, &namespace)) {
		if (!String_Parse($("%:%"), value, &src, &dest)) {
			Logger_Error(this->logger,
				$("`map' requires two or three values separated by a colon."));
			return false;
		}

		namespace = $("main");
	}

	if (src.len == 0) {
		Logger_Error(this->logger, $("Invalid source path."));
		return false;
	}

	if (!Path_isFolderPath(dest)) {
		Logger_Error(this->logger,
			$("Destination path '%' is invalid."), dest);
		return false;
	}

	if (!Path_exists(dest)) {
		Logger_Error(this->logger,
			$("Destination path '%' does not exist."), dest);

		Terminal_Prompt prompt;
		Terminal_Prompt_Init(&prompt, this->term);

		bool isYes = Terminal_Prompt_Ask(&prompt, $$("Create path?"));

		Terminal_Prompt_Destroy(&prompt);

		if (isYes) {
			Path_createFolder(dest, true);
		} else {
			return false;
		}
	}

	DepsMapping insert = {
		.src  = String_Clone(src),
		.dest = String_Clone(dest),
		.namespace = String_Clone(namespace)
	};

	MappingArray_Push(&this->mappings, insert);

	return true;
}

def(void, setLibrary, bool value) {
	this->library = value;
}

def(void, setOutput, RdString value) {
	String_Copy(&this->output, value);
}

def(void, setRuntime, RdString value) {
	String_Copy(&this->runtime, value);
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
		String realPath    = Path_expand(shortPath);

		if (String_BeginsWith(path, realPath.rd)) {
			String res = String_New(0);

			if (!String_Equals(shortPath, $("./"))) {
				String_Append(&res, shortPath);
			}

			String_Append(&res, String_Slice(path, realPath.len));

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

	if (this->library) {
		String soname =
			String_Format($("-Wl,-soname,%"),
				Path_getFileName(this->output.rd));
		Process_addParameter(&proc, soname.rd);
		String_Destroy(&soname);

		Process_addParameter(&proc, $("-shared"));
	}

	Process_addParameter(&proc, $("-o"));
	Process_addParameter(&proc, this->output.rd);

	fwd(i, files->len) {
		Process_addParameter(&proc, files->buf[i].rd);
	}

	Process_addParameter(&proc, $("-L"));
	Process_addParameter(&proc, this->runtime.rd);

	Process_addParameter(&proc, $("-B"));
	Process_addParameter(&proc, this->runtime.rd);

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

static def(pid_t, compile, RdString namespace, RdString src, RdString out) {
	Process proc = Process_new(this->cc.rd);

	if (this->library) {
		Process_addParameter(&proc, $("-fpic"));
	}

	Process_addParameter(&proc, $("-o"));
	Process_addParameter(&proc, out);

	assert(!String_Contains(namespace, ' '));
	String namesp = String_Format($("-DNamespace=\"%\""), namespace);
	Process_addParameter(&proc, namesp.rd);
	String_Destroy(&namesp);

	Process_addParameter(&proc, $("-c"));
	Process_addParameter(&proc, src);

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

		RdString create = Path_getFolderPath(item->output.rd);
		if (!Path_exists(create)) {
			Path_createFolder(create, true);
		}

		String path = call(shrinkPath, item->source);

		String strCur   = Integer_ToString(Queue_getOffset(&this->queue));
		String strTotal = Integer_ToString(Queue_getTotal(&this->queue));

		Logger_Info(this->logger, $("Compiling %... [%/%]"),
			path.rd, strCur.rd, strTotal.rd);

		String_Destroy(&strCur);
		String_Destroy(&strTotal);

		RdString namespace = item->namespace;
		RdString output    = item->output.rd;

		pid_t pid = call(compile, namespace, path.rd, output);
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

	fwd(i, this->mappings->len) {
		RdString path = this->mappings->buf[i].src.rd;

		rpt(2) {
			if (!Path_isFolderPath(path) || !Path_exists(path)) {
				Logger_Error(this->logger,
					$("Mapped path '%' is invalid. Trailing slash missing?"), path);
				return;
			}

			path = this->mappings->buf[i].dest.rd;
		}
	}

	if (!Path_isFolderPath(this->runtime.rd) || !Path_exists(this->runtime.rd)) {
		Logger_Error(this->logger,
			$("The runtime path '%' is invalid. Trailing slash missing?"),
			this->runtime.rd);
		return;
	}

	if (this->mappings->len == 0) {
		Logger_Error(this->logger, $("No mappings defined."));
		return;
	}

	if (this->output.len == 0) {
		RdString main = Deps_getMain(this->deps);
		RdString ext  = Path_getFileExtension(main);

		if (ext.len == 0) {
			String_Copy(&this->output, $("a.out"));
		} else {
			String_Append(&this->output, FmtString($("%.exe"),
				String_Slice(main, 0, -ext.len - 1)));
		}
	}

	Logger_Info(this->logger, $("Writing to %."), this->output.rd);

	if (!Path_isFilePath(this->output.rd)) {
		Logger_Error(this->logger, $("Output path does not point to a file."));
		return;
	}

	RdString outputPath = Path_getFolderPath(this->output.rd);

	if (!Path_exists(outputPath) || !Path_isWritable(outputPath)) {
		Logger_Error(this->logger, $("Output path is not writable."));
		return;
	}

	this->queue = Queue_new(this->logger, this->deps, this->mappings);

	Queue_create(&this->queue);
	Queue_purge(&this->queue);

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
