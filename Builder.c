#import "Builder.h"

#define self Builder

def(void, Init, Logger *logger, Deps *deps) {
	this->deps      = deps;
	this->logger    = logger;
	this->output    = String_Clone($("a.out"));
	this->cc        = String_Clone($("/usr/bin/clang"));
	this->inclhdr   = String_New(0);
	this->manifest  = String_New(0);
	this->dbgsym    = false;
	this->std       = String_Clone($("gnu99"));
	this->blocks    = true;
	this->optmlevel = 0;
	this->verbose   = false;
	this->link      = StringArray_New(0);
	this->queue     = QueueArray_New(0);
	this->mappings  = MappingArray_New(0);
	this->linkpaths = StringArray_New(0);
}

def(void, Destroy) {
	String_Destroy(&this->output);
	String_Destroy(&this->cc);
	String_Destroy(&this->inclhdr);
	String_Destroy(&this->manifest);
	String_Destroy(&this->std);

	StringArray_Destroy(this->link);
	StringArray_Free(this->link);

	StringArray_Destroy(this->linkpaths);
	StringArray_Free(this->linkpaths);

	each(item, this->queue) {
		String_Destroy(&item->source);
		String_Destroy(&item->output);
	}

	each(item, this->mappings) {
		String_Destroy(&item->src);
		String_Destroy(&item->dest);
	}

	MappingArray_Free(this->mappings);
	QueueArray_Free(this->queue);
}

static def(bool, Map, RdString value) {
	bool src = true;
	RdString s = $("");
	ref(DepsMapping) insert;

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
		goto error;
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
		String_Copy(&this->manifest, value);
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

static def(String, GetOutput, RdString path) {
	String realpath = Path_Resolve(path);

	fwd(i, this->mappings->len) {
		String mapping = Path_Resolve(this->mappings->buf[i].src.rd);

		if (String_BeginsWith(realpath.rd, mapping.rd)) {
			String out = String_Clone(this->mappings->buf[i].dest.rd);
			String_Append(&out, String_Slice(realpath.rd, mapping.len));

			if (String_EndsWith(out.rd, $(".cpp"))) {
				String_Crop(&out, 0, -4);
			} else if (String_EndsWith(out.rd, $(".c"))) {
				String_Crop(&out, 0, -2);
			}

			String_Append(&out, $(".o"));

			String_Destroy(&realpath);
			String_Destroy(&mapping);

			return out;
		}

		String_Destroy(&mapping);
	}

	String_Destroy(&realpath);

	return String_New(0);
}

static sdef(String, GetSource, RdString path) {
	ssize_t pos = String_ReverseFind(path, '.');

	if (pos == String_NotFound) {
		return String_New(0);
	}

	RdString ext = String_Slice(path, pos);

	if (String_Equals(ext, $("c")) ||
		String_Equals(ext, $("cpp")))
	{
		/* Already a source file. */
		return String_Clone(path);
	}

	String res = String_Clone(String_Slice(path, 0, pos + 1));

	String_Append(&res, 'c');

	if (!Path_Exists(res.rd)) {
		String_Append(&res, $("pp"));

		if (!Path_Exists(res.rd)) {
			res.len = 0;
		}
	}

	return res;
}

static def(void, AddToQueue, RdString source, RdString output) {
	fwd(i, this->queue->len) {
		if (String_Equals(this->queue->buf[i].source.rd, source)) {
			return;
		}
	}

	ref(QueueItem) item = {
		.source = String_Clone(source),
		.output = String_Clone(output)
	};

	QueueArray_Push(&this->queue, item);
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
	Process_AddParameter(&proc, $("-pipe"));
	Process_AddParameter(&proc, $("-Wshorten-64-to-32"));

	if (this->manifest.len != 0) {
		Process_AddParameter(&proc, $("-include"));
		Process_AddParameter(&proc, this->manifest.rd);
	}

	if (this->inclhdr.len > 0) {
		Process_AddParameter(&proc, $("-include"));
		Process_AddParameter(&proc, this->inclhdr.rd);
	}

	StringArray *deps = Deps_GetIncludes(this->deps);

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

def(bool, Traverse, Deps_Component *node, size_t depth) {
	bool build = false;

	String prefix = String_New(depth * 2);
	rpt(depth * 2) {
		String_Append(&prefix, ' ');
	}

	String headerPath = String_Clone(node->path.rd);
	String sourcePath = scall(GetSource, headerPath.rd);

	/* Skip all non-source files. */
	if (sourcePath.len == 0) {
		Logger_Debug(this->logger,
			$("%'%' has no corresponding source file"),
			prefix.rd, headerPath.rd);
	} else {
		Logger_Info(this->logger, $("%Processing %..."), prefix.rd, sourcePath.rd);

		String output = call(GetOutput, sourcePath.rd);

		if (output.len == 0) {
			Logger_Error(this->logger,
				$("%No output path for '%' is mapped."),
				prefix.rd, sourcePath.rd);

			String_Destroy(&headerPath);
			String_Destroy(&sourcePath);

			throw(RuntimeError);
		}

		if (!Path_Exists(output.rd)) {
			Logger_Info(this->logger, $("%Not built yet."), prefix.rd);
			call(AddToQueue, sourcePath.rd, output.rd);
			build = true;
		} else if (File_IsModified(sourcePath.rd, output.rd)) {
			Logger_Info(this->logger, $("%Source modified."), prefix.rd);
			call(AddToQueue, sourcePath.rd, output.rd);
			build = true;
		} else if (File_IsModified(headerPath.rd, output.rd)) {
			Logger_Info(this->logger, $("%Header modified."), prefix.rd);
			call(AddToQueue, sourcePath.rd, output.rd);
			build = true;
		}

		String_Destroy(&output);
	}

	fwd(i, node->len) {
		if (call(Traverse, node->buf[i], depth + 1)) {
			build = true;
		}
	}

	if (!build) {
		Logger_Debug(this->logger, $("%Not building."), prefix.rd);
	}

	String_Destroy(&sourcePath);
	String_Destroy(&headerPath);
	String_Destroy(&prefix);

	return build;
}

def(bool, CreateQueue) {
	Deps_Component *comps = Deps_GetComponent(this->deps);

	each(comp, comps) {
		call(Traverse, *comp, 0);
	}

	return true;
}

def(void, PrintQueue) {
	if (this->queue->len == 0 && Path_Exists(this->output.rd)) {
		Logger_Info(this->logger, $("  Queue is empty."));
	} else {
		Logger_Info(this->logger, $("  Queue:"));

		fwd(i, this->queue->len) {
			Logger_Info(this->logger, $(" - % --> %"),
				this->queue->buf[i].source.rd,
				this->queue->buf[i].output.rd);
		}

		Logger_Info(this->logger, $(" - % (link)"), this->output.rd);
	}
}

def(void, CreateManifest) {
	Deps_Modules *modules = Deps_GetModules(this->deps);

	File file = File_New(this->manifest.rd,
		FileStatus_Create    |
		FileStatus_WriteOnly |
		FileStatus_Truncate);
	File_Write(&file, $("enum {\n"));

	each(module, modules) {
		String fmt = String_Format($("\tModules_%,\n"), module->name.rd);
		File_Write(&file, fmt.rd);
		String_Destroy(&fmt);

		each(exc, module->exc) {
			String fmt2 = String_Format($("\t%_%,\n"), module->name.rd, exc->rd);
			File_Write(&file, fmt2.rd);
			String_Destroy(&fmt2);
		}

		String fmt3 = String_Format($("\tModules_%_Length,\n"), module->name.rd);
		File_Write(&file, fmt3.rd);
		String_Destroy(&fmt3);
	}

	File_Write(&file, $(
		"};\n"
		"\n"
		"static char* codes[] = {\n"
	));

	each(module, modules) {
		each(exc, module->exc) {
			String fmt = String_Format($("\t[%_%] = \"%\",\n"),
				module->name.rd, exc->rd, exc->rd);
			File_Write(&file, fmt.rd);
			String_Destroy(&fmt);
		}
	}

	File_Write(&file, $(
		"};\n"
		"\n"
		"static inline char* Manifest_ResolveCode(unsigned int code) {\n"
		"\tif (code > sizeof(codes) / sizeof(codes[0])) {\n"
		"\t\treturn \"\";\n"
		"\t}\n"
		"\n"
		"\treturn codes[code];\n"
		"}\n"
		"\n"
		"static inline char* Manifest_ResolveName(int module) {\n"
		"\tswitch (module) {\n"));

	each(module, modules) {
		String readable = String_ReplaceAll(module->name.rd,
			$("_"),
			$("."));

		String fmt = String_Format($(
			"\t\tcase Modules_% ... Modules_%_Length:\n"
			"\t\t\treturn \"%\";\n"),
			module->name.rd,
			module->name.rd,
			readable.rd);

		String_Destroy(&readable);

		File_Write(&file, fmt.rd);
		String_Destroy(&fmt);
	}

	File_Write(&file, $(
		"\t}\n"
		"\n"
		"\treturn \"Unknown module\";\n"
		"}\n"));

	File_Destroy(&file);

	Logger_Info(this->logger,
		$("Manifest written to '%'."),
		this->manifest.rd);
}

def(bool, Run) {
	if (this->manifest.len != 0) {
		call(CreateManifest);
	}

	if (this->mappings->len == 0) {
		return true;
	}

	if (!call(CreateQueue)) {
		return false;
	}

	if (this->queue->len != 0 || !Path_Exists(this->output.rd)) {
		fwd(i, this->queue->len) {
			RdString create = Path_GetDirectory(this->queue->buf[i].output.rd);

			if (!Path_Exists(create)) {
				Path_Create(create, true);
			}

			String path = call(ShrinkPath, this->queue->buf[i].source.rd);

			String strCur   = Integer_ToString(i + 1);
			String strTotal = Integer_ToString(this->queue->len);

			Logger_Info(this->logger, $("Compiling %... [%/%]"),
				path.rd, strCur.rd, strTotal.rd);

			String_Destroy(&strCur);
			String_Destroy(&strTotal);

			bool ok = call(Compile, path, this->queue->buf[i].output);

			String_Destroy(&path);

			if (!ok) {
				return false;
			}
		}

		StringArray *files = StringArray_New(0);

		StringArray *paths = Deps_GetPaths(this->deps);

		fwd(i, paths->len) {
			String src = scall(GetSource, paths->buf[i].rd);

			if (src.len > 0) {
				String path = call(GetOutput, src.rd);

				if (path.len == 0) {
					StringArray_Destroy(files);
					StringArray_Free(files);

					String_Destroy(&src);

					return false;
				}

				String shrinked = call(ShrinkPath, path.rd);

				if (StringArray_Contains(files, shrinked.rd)) {
					String_Destroy(&shrinked);
				} else {
					StringArray_Push(&files, shrinked);
				}

				String_Destroy(&path);
			}

			String_Destroy(&src);
		}

		call(Link, files);

		StringArray_Destroy(files);
		StringArray_Free(files);
	}

	return true;
}
