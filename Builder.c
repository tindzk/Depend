#import "Builder.h"

#define self Builder

def(void, Init, Terminal *term, Logger *logger, Deps *deps) {
	this->term      = term;
	this->deps      = deps;
	this->logger    = logger;
	this->output    = String_Clone($("a.out"));
	this->cc        = String_Clone($("/usr/bin/clang"));
	this->inclhdr   = String_New(0);
	this->manifest  = false;
	this->dbgsym    = false;
	this->std       = String_Clone($("gnu99"));
	this->blocks    = true;
	this->optmlevel = 0;
	this->verbose   = false;
	this->link      = StringArray_New(0);
	this->mappings  = MappingArray_New(0);
	this->linkpaths = StringArray_New(0);
}

def(void, Destroy) {
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

static def(bool, Map, RdString value) {
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

def(bool, SetOption, RdString name, RdString value) {
	if (String_Equals(name, $("output"))) {
		String_Copy(&this->output, value);
	} else if (String_Equals(name, $("map"))) {
		return call(Map, value);
	} else if (String_Equals(name, $("cc"))) {
		String_Copy(&this->cc, value);
	} else if (String_Equals(name, $("inclhdr"))) {
		String_Copy(&this->inclhdr, value);
	} else if (String_Equals(name, $("manifest"))) {
		this->manifest = true;
	} else if (String_Equals(name, $("dbgsym"))) {
		this->dbgsym = String_Equals(value, $("yes"));
	} else if (String_Equals(name, $("std"))) {
		String_Copy(&this->std, value);
	} else if (String_Equals(name, $("blocks"))) {
		this->blocks = String_Equals(value, $("yes"));
	} else if (String_Equals(name, $("optimlevel"))) {
		this->optmlevel = Int16_Parse(value);
	} else if (String_Equals(name, $("link"))) {
		StringArray_Push(&this->link, String_Clone(value));
	} else if (String_Equals(name, $("linkpath"))) {
		StringArray_Push(&this->linkpaths, String_Clone(value));
	} else if (String_Equals(name, $("verbose"))) {
		this->verbose = true;
	}

	return true;
}

static sdef(String, ShrinkPathEx, RdString shortpath, RdString path) {
	String realpath = Path_Resolve(shortpath);

	String res = String_New(0);

	if (String_BeginsWith(path, realpath.rd)) {
		if (!String_Equals(shortpath, $("."))) {
			String_Append(&res, shortpath);
			String_Append(&res, '/');
		}

		String_Append(&res, String_Slice(path, realpath.len + 1));
	}

	String_Destroy(&realpath);

	return res;
}

static def(String, ShrinkPath, RdString path) {
	each(mapping, this->mappings) {
		String shortpath = mapping->src;
		String res = scall(ShrinkPathEx, shortpath.rd, path);

		if (res.len > 0) {
			return res;
		}
	}

	return String_Clone(path);
}

static def(bool, Compile, String src, String out) {
	Process proc = Process_New(this->cc.rd);

	Process_AddParameter(&proc, $("-o"));
	Process_AddParameter(&proc, out.rd);

	Process_AddParameter(&proc, $("-c"));
	Process_AddParameter(&proc, src.rd);

	String std = String_Concat($("-std="), this->std.rd);
	Process_AddParameter(&proc, std.rd);
	String_Destroy(&std);

	if (this->blocks) {
		Process_AddParameter(&proc, $("-fblocks"));
	}

	String strLevel = Integer_ToString(this->optmlevel);
	String optim    = String_Format($("-O%"), strLevel.rd);

	Process_AddParameter(&proc, optim.rd);

	String_Destroy(&optim);
	String_Destroy(&strLevel);

	if (this->dbgsym) {
		Process_AddParameter(&proc, $("-g"));
	}

	Process_AddParameter(&proc, $("-W"));
	Process_AddParameter(&proc, $("-Wall"));
	Process_AddParameter(&proc, $("-Wextra"));
	Process_AddParameter(&proc, $("-Wattributes"));
	Process_AddParameter(&proc, $("-Wbad-function-cast"));
	Process_AddParameter(&proc, $("-Wcast-align"));
	Process_AddParameter(&proc, $("-Wcast-qual"));
	Process_AddParameter(&proc, $("-Wchar-subscripts"));
	Process_AddParameter(&proc, $("-Wdeclaration-after-statement"));
	Process_AddParameter(&proc, $("-Wdisabled-optimization"));
	Process_AddParameter(&proc, $("-Werror-implicit-function-declaration"));
	Process_AddParameter(&proc, $("-Wfatal-errors"));
	Process_AddParameter(&proc, $("-Wfloat-equal"));
	Process_AddParameter(&proc, $("-Winit-self"));
	Process_AddParameter(&proc, $("-Winline"));
	Process_AddParameter(&proc, $("-Wmissing-declarations"));
	Process_AddParameter(&proc, $("-Wmissing-field-initializers"));
	Process_AddParameter(&proc, $("-Wmissing-format-attribute"));
	Process_AddParameter(&proc, $("-Wmissing-include-dirs"));
	Process_AddParameter(&proc, $("-Wmissing-noreturn"));
	Process_AddParameter(&proc, $("-Wnested-externs"));
	Process_AddParameter(&proc, $("-Wold-style-definition"));
	Process_AddParameter(&proc, $("-Wpacked"));
	Process_AddParameter(&proc, $("-Wparentheses"));
	Process_AddParameter(&proc, $("-Wpointer-sign"));
	Process_AddParameter(&proc, $("-Wredundant-decls"));
	Process_AddParameter(&proc, $("-Wreturn-type"));
	Process_AddParameter(&proc, $("-Wsequence-point"));
	Process_AddParameter(&proc, $("-Wshadow"));
	Process_AddParameter(&proc, $("-Wshorten-64-to-32"));
	Process_AddParameter(&proc, $("-Wsign-compare"));
	Process_AddParameter(&proc, $("-Wstrict-prototypes"));
	Process_AddParameter(&proc, $("-Wswitch"));
	Process_AddParameter(&proc, $("-Wswitch-default"));
	Process_AddParameter(&proc, $("-Wswitch-enum"));
	Process_AddParameter(&proc, $("-Wundef"));
	Process_AddParameter(&proc, $("-Wuninitialized"));
	Process_AddParameter(&proc, $("-Wunused"));
	Process_AddParameter(&proc, $("-Wunused-parameter"));
	Process_AddParameter(&proc, $("-Wunused-value"));
	Process_AddParameter(&proc, $("-Wwrite-strings"));

	Process_AddParameter(&proc, $("-fstrict-aliasing"));
	Process_AddParameter(&proc, $("-Wstrict-aliasing=2"));

	Process_AddParameter(&proc, $("-fstrict-overflow"));
	Process_AddParameter(&proc, $("-Wstrict-overflow=5"));

	Process_AddParameter(&proc, $("-funit-at-a-time"));
	Process_AddParameter(&proc, $("-fno-omit-frame-pointer"));

	Process_AddParameter(&proc, $("-pipe"));

	if (this->inclhdr.len > 0) {
		Process_AddParameter(&proc, $("-include"));
		Process_AddParameter(&proc, this->inclhdr.rd);
	}

	StringArray *deps = Deps_getIncludes(this->deps);

	fwd(i, deps->len) {
		Process_AddParameter(&proc, $("-I"));
		Process_AddParameter(&proc, deps->buf[i].rd);
	}

	if (this->verbose) {
		String cmd = Process_GetCommandLine(&proc);
		Logger_Info(this->logger, cmd.rd);
		String_Destroy(&cmd);
	}

	int res = Process_Spawn(&proc);

	Process_Destroy(&proc);

	return res < 0;
}

static def(void, Link, StringArray *files) {
	Process proc = Process_New(this->cc.rd);

	Process_AddParameter(&proc, $("-o"));
	Process_AddParameter(&proc, this->output.rd);

	fwd(i, files->len) {
		Process_AddParameter(&proc, files->buf[i].rd);
	}

	fwd(i, this->linkpaths->len) {
		Process_AddParameter(&proc, $("-L"));
		Process_AddParameter(&proc, this->linkpaths->buf[i].rd);
	}

	if (this->dbgsym) {
		Process_AddParameter(&proc, $("-g"));
	}

	fwd(i, this->link->len) {
		if (this->link->buf[i].len == 0) {
			continue;
		}

		if (this->link->buf[i].buf[0] == '@') {
			Process_AddParameter(&proc, $("-Wl,-Bdynamic"));
		} else {
			Process_AddParameter(&proc, $("-Wl,-Bstatic"));
		}

		Process_AddParameter(&proc, $("-l"));
		Process_AddParameter(&proc, String_Slice(
			this->link->buf[i].rd,
			this->link->buf[i].buf[0] == '@'));
	}

	Process_Spawn(&proc);

	if (this->verbose) {
		String cmd = Process_GetCommandLine(&proc);
		Logger_Info(this->logger, cmd.rd);
		String_Destroy(&cmd);
	}

	Process_Destroy(&proc);
}

def(bool, Run) {
	if (this->manifest) {
		ManifestWriter manifest =
			ManifestWriter_new(this->logger, this->deps);
		ManifestWriter_create(&manifest);
		ManifestWriter_destroy(&manifest);
	}

	if (this->mappings->len == 0) {
		return true;
	}

	Queue queue = Queue_new(this->logger, this->deps, this->mappings);
	Queue_create(&queue);

	size_t cnt = 1;

	while (Queue_hasNext(&queue)) {
		Queue_Item item = Queue_getNext(&queue);

		RdString create = Path_GetDirectory(item.output.rd);

		if (!Path_Exists(create)) {
			Path_Create(create, true);
		}

		String path = call(ShrinkPath, item.source);

		String strCur   = Integer_ToString(cnt);
		String strTotal = Integer_ToString(Queue_getTotal(&queue));

		Logger_Info(this->logger, $("Compiling %... [%/%]"),
			path.rd, strCur.rd, strTotal.rd);

		String_Destroy(&strCur);
		String_Destroy(&strTotal);

		bool ok = call(Compile, path, item.output);

		String_Destroy(&path);

		if (!ok) {
			return false;
		}

		cnt++;
	}

	StringArray *files = Queue_getLinkingFiles(&queue);

	call(Link, files);

	StringArray_Destroy(files);
	StringArray_Free(files);

	Queue_destroy(&queue);

	return true;
}
