#import "Builder.h"

#define self Builder

extern Logger logger;

def(void, Init, DepsInstance deps) {
	this->deps      = deps;
	this->output    = String_Clone($("a.out"));
	this->cc        = String_Clone($("/usr/bin/clang"));
	this->inclhdr   = HeapString(0);
	this->manifest  = HeapString(0);
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

	foreach (item, this->queue) {
		String_Destroy(&item->source);
		String_Destroy(&item->output);
	}

	foreach (item, this->mappings) {
		String_Destroy(&item->src);
		String_Destroy(&item->dest);
	}

	MappingArray_Free(this->mappings);
	QueueArray_Free(this->queue);
}

def(bool, Map, String value) {
	StringArray *parts = String_Split(value, ':');

	if (parts->len < 2) {
		Logger_Error(&logger,
			$("`map' requires two values separated by a colon."));

		goto error;
	}

	ref(DepsMapping) insert = {
		.src  = parts->buf[0],
		.dest = parts->buf[1]
	};

	if (insert.src.len == 0) {
		Logger_Error(&logger, $("Invalid source path."));
		goto error;
	}

	if (!Path_Exists(insert.dest)) {
		Logger_Error(&logger,
			$("Destination path '%' does not exist."),
			insert.dest);

		goto error;
	}

	insert.src  = String_Clone(insert.src);
	insert.dest = String_Clone(insert.dest);

	MappingArray_Push(&this->mappings, insert);

	bool res = true;

	when (error) {
		res = false;
	}

	StringArray_Free(parts);

	return res;
}

def(bool, SetOption, String name, String value) {
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

static sdef(String, ShrinkPathEx, String shortpath, String path) {
	String realpath = Path_Resolve(shortpath);

	String res = HeapString(0);

	if (String_BeginsWith(path, realpath)) {
		if (!String_Equals(shortpath, $("."))) {
			String_Append(&res, shortpath);
			String_Append(&res, '/');
		}

		String_Append(&res, String_Slice(path, realpath.len + 1));
	}

	String_Destroy(&realpath);

	return res;
}

static def(String, ShrinkPath, String path) {
	foreach (mapping, this->mappings) {
		String shortpath = mapping->src;
		String res = scall(ShrinkPathEx, shortpath, path);

		if (res.len > 0) {
			return res;
		}
	}

	return String_Clone(path);
}

static def(String, GetOutput, String path) {
	String realpath = Path_Resolve(path);

	forward (i, this->mappings->len) {
		String mapping = Path_Resolve(this->mappings->buf[i].src);

		if (String_BeginsWith(realpath, mapping)) {
			String out = String_Clone(this->mappings->buf[i].dest);
			String_Append(&out, String_Slice(realpath, mapping.len));

			if (String_EndsWith(out, $(".cpp"))) {
				String_Crop(&out, 0, -4);
			} else if (String_EndsWith(out, $(".c"))) {
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

	return HeapString(0);
}

static sdef(String, GetSource, String path) {
	ssize_t pos = String_ReverseFind(path, '.');

	if (pos == String_NotFound) {
		return HeapString(0);
	}

	String ext = String_Slice(path, pos);

	if (String_Equals(ext, $("c"))
	 || String_Equals(ext, $("cpp")))
	{
		/* Already a source file. */
		return String_Clone(path);
	}

	String res = String_Clone(String_Slice(path, 0, pos + 1));

	String_Append(&res, 'c');

	if (!Path_Exists(res)) {
		String_Append(&res, $("pp"));

		if (!Path_Exists(res)) {
			res.len = 0;
		}
	}

	return res;
}

static def(void, AddToQueue, String source, String output) {
	forward (i, this->queue->len) {
		if (String_Equals(this->queue->buf[i].source, source)) {
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
	Process proc;
	Process_Init(&proc, this->cc);

	Process_AddParameter(&proc, $("-o"));
	Process_AddParameter(&proc, out);

	Process_AddParameter(&proc, $("-c"));
	Process_AddParameter(&proc, src);

	String std = String_Concat($("-std="), this->std);
	Process_AddParameter(&proc, std);
	String_Destroy(&std);

	if (this->blocks) {
		Process_AddParameter(&proc, $("-fblocks"));
	}

	String optim = String_Format($("-O%"), Int16_ToString(this->optmlevel));
	Process_AddParameter(&proc, optim);
	String_Destroy(&optim);

	if (this->dbgsym) {
		Process_AddParameter(&proc, $("-g"));
	}

	Process_AddParameter(&proc, $("-W"));
	Process_AddParameter(&proc, $("-Wall"));
	Process_AddParameter(&proc, $("-pipe"));
	Process_AddParameter(&proc, $("-Wshorten-64-to-32"));

	if (this->manifest.len != 0) {
		Process_AddParameter(&proc, $("-include"));
		Process_AddParameter(&proc, this->manifest);
	}

	if (this->inclhdr.len > 0) {
		Process_AddParameter(&proc, $("-include"));
		Process_AddParameter(&proc, this->inclhdr);
	}

	StringArray *deps = Deps_GetIncludes(this->deps);

	forward (i, deps->len) {
		Process_AddParameter(&proc, $("-I"));
		Process_AddParameter(&proc, deps->buf[i]);
	}

	if (this->verbose) {
		String cmd = Process_GetCommandLine(&proc);
		Logger_Info(&logger, cmd);
		String_Destroy(&cmd);
	}

	int res = Process_Spawn(Process_FromObject(&proc));

	Process_Destroy(&proc);

	return res < 0;
}

static def(void, Link, StringArray *files) {
	Process proc;
	Process_Init(&proc, this->cc);

	Process_AddParameter(&proc, $("-o"));
	Process_AddParameter(&proc, this->output);

	forward (i, files->len) {
		Process_AddParameter(&proc, files->buf[i]);
	}

	forward (i, this->linkpaths->len) {
		Process_AddParameter(&proc, $("-L"));
		Process_AddParameter(&proc, this->linkpaths->buf[i]);
	}

	if (this->dbgsym) {
		Process_AddParameter(&proc, $("-g"));
	}

	forward (i, this->link->len) {
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
			this->link->buf[i],
			this->link->buf[i].buf[0] == '@'));
	}

	Process_Spawn(Process_FromObject(&proc));

	if (this->verbose) {
		String cmd = Process_GetCommandLine(&proc);
		Logger_Info(&logger, cmd);
		String_Destroy(&cmd);
	}

	Process_Destroy(&proc);
}

def(bool, CreateQueue) {
	DepsArray *deps = Deps_GetDeps(this->deps);

	forward (i, deps->len) {
		Deps_Node *dep = deps->buf[i];

		String headerPath = String_Clone(dep->path);
		String sourcePath = scall(GetSource, headerPath);

		/* Skip all non-source files. */
		if (sourcePath.len == 0) {
			Logger_Debug(&logger,
				$("'%' has no corresponding source file"),
				headerPath);

			String_Destroy(&headerPath);
			String_Destroy(&sourcePath);

			continue;
		}

		Logger_Info(&logger, $("Processing %..."), sourcePath);

		String output = call(GetOutput, sourcePath);

		if (output.len == 0) {
			Logger_Error(&logger,
				$("No output path for '%' is mapped."),
				sourcePath);

			return false;
		}

		/* Will be true when at least one dependency has changed. */
		bool depChanged = false;

		if (dep->len > 0) {
			Logger_Debug(&logger, $("Depends on:"));

			forward (j, dep->len) {
				Logger_Debug(&logger, $(" - %"), dep->buf[j]->path);

				String depHeaderPath = String_Clone(dep->buf[j]->path);
				String depSourcePath = scall(GetSource, dep->buf[j]->path);

				if (depSourcePath.len == 0) { /* Header file wihout matching source file. */
					if (Path_Exists(output)) {
						if (File_IsModified(depHeaderPath, output)) {
							Logger_Info(&logger, $("Dependency header changed."));
							depChanged = true;
						}
					}
				} else { /* There is a source file. */
					String depOutput = call(GetOutput, depSourcePath);

					if (depOutput.len == 0) {
						Logger_Error(&logger,
							$("No output path for '%' is mapped."),
							depSourcePath);

						return false;
					}

					if (!Path_Exists(depOutput)) {
						/* Dependency unbuilt. */
						call(AddToQueue, depSourcePath, depOutput);
						depChanged = true;
					} else if (File_IsModified(depSourcePath, depOutput)) {
						/* Dependency source changed. */
						call(AddToQueue, depSourcePath, depOutput);
						depChanged = true;
					} else if (File_IsModified(depHeaderPath, depOutput)) {
						/* Dependency header changed. */
						call(AddToQueue, depSourcePath, depOutput);
						depChanged = true;
					}

					String_Destroy(&depOutput);
				}

				String_Destroy(&depSourcePath);
				String_Destroy(&depHeaderPath);
			}
		}

		if (depChanged) {
			Logger_Info(&logger, $("Dependency changed or unbuilt."));
			call(AddToQueue, sourcePath, output);
		} else if (!Path_Exists(output)) {
			Logger_Info(&logger, $("Not built yet."));
			call(AddToQueue, sourcePath, output);
		} else if (File_IsModified(sourcePath, output)) {
			Logger_Info(&logger, $("Source modified."));
			call(AddToQueue, sourcePath, output);
		} else if (File_IsModified(headerPath, output)) {
			Logger_Info(&logger, $("Header modified."));
			call(AddToQueue, sourcePath, output);
		} else {
			Logger_Debug(&logger, $("Already built."));
		}

		String_Destroy(&sourcePath);
		String_Destroy(&headerPath);
		String_Destroy(&output);
	}

	return true;
}

def(void, PrintQueue) {
	if (this->queue->len == 0 && Path_Exists(this->output)) {
		Logger_Info(&logger, $("  Queue is empty."));
	} else {
		Logger_Info(&logger, $("  Queue:"));

		forward (i, this->queue->len) {
			Logger_Info(&logger, $(" - % --> %"),
				this->queue->buf[i].source,
				this->queue->buf[i].output);
		}

		Logger_Info(&logger, $(" - % (link)"), this->output);
	}
}

def(void, CreateManifest) {
	size_t counter = 1;

	DepsArray *deps = Deps_GetDeps(this->deps);

	File file;
	File_Open(&file, this->manifest,
		FileStatus_Create    |
		FileStatus_WriteOnly |
		FileStatus_Truncate);

	String fmt = String_Format($("#define Manifest_GapSize %\n"),
		Int16_ToString(
			Builder_ManifestGapSize));

	File_Write(&file, fmt);

	String_Destroy(&fmt);

	forward (i, deps->len) {
		foreach (module, deps->buf[i]->modules) {
			String fmt = String_Format($("#define Modules_% %\n"),
				*module,
				Integer_ToString(
					(u32) Builder_ManifestGapSize * counter));

			File_Write(&file, fmt);

			String_Destroy(&fmt);

			counter++;
		}
	}

	File_Write(&file, $("\n"));
	File_Write(&file, $(
		"static inline char* Manifest_ResolveName(int module) {\n"
		"\tswitch (module) {\n"));

	forward (i, deps->len) {
		foreach (module, deps->buf[i]->modules) {
			String readable = String_ReplaceAll(*module,
				$("_"),
				$("."));

			String fmt = String_Format($(
				"\t\tcase Modules_% ... Modules_% + Manifest_GapSize - 1:\n"
				"\t\t\treturn \"%\";\n"),
				*module,
				*module,
				readable);

			File_Write(&file, fmt);
			String_Destroy(&fmt);
		}
	}

	File_Write(&file, $(
		"\t}\n"
		"\n"
		"\treturn \"Unknown module\";\n"
		"}\n"));

	File_Close(&file);

	Logger_Info(&logger,
		$("Manifest written to '%'."),
		this->manifest);
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

	if (this->queue->len != 0 || !Path_Exists(this->output)) {
		forward (i, this->queue->len) {
			String create = Path_GetDirectory(this->queue->buf[i].output);

			if (!Path_Exists(create)) {
				Path_Create(create, true);
			}

			String path = call(ShrinkPath, this->queue->buf[i].source);

			Logger_Info(&logger, $("Compiling %... [%/%]"),
				path,
				Int32_ToString(i + 1),
				Int32_ToString(this->queue->len));

			bool ok = call(Compile, path, this->queue->buf[i].output);

			String_Destroy(&path);

			if (!ok) {
				return false;
			}
		}

		StringArray *files = StringArray_New(0);

		DepsArray *deps = Deps_GetDeps(this->deps);

		forward (i, deps->len) {
			String src = scall(GetSource, deps->buf[i]->path);

			if (src.len > 0) {
				String path = call(GetOutput, src);

				if (path.len == 0) {
					foreach (file, files) {
						String_Destroy(file);
					}

					StringArray_Free(files);

					String_Destroy(&src);

					return false;
				}

				String shrinked = call(ShrinkPath, path);

				if (StringArray_Contains(files, shrinked)) {
					String_Destroy(&shrinked);
				} else {
					StringArray_Push(&files, shrinked);
				}

				String_Destroy(&path);
			}

			String_Destroy(&src);
		}

		call(Link, files);

		foreach (file, files) {
			String_Destroy(file);
		}

		StringArray_Free(files);
	}

	return true;
}
