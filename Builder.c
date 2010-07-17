#include "Builder.h"

extern Logger logger;

void Builder_Init(Builder *this, Deps *deps) {
	this->deps      = deps;
	this->output    = String_Clone($("a.out"));
	this->cc        = String_Clone($("/usr/bin/clang"));
	this->inclhdr   = HeapString(0);
	this->dbgsym    = false;
	this->std       = String_Clone($("gnu99"));
	this->blocks    = true;
	this->optmlevel = 0;
	this->verbose   = false;

	Array_Init(&this->link,      0);
	Array_Init(&this->queue,     0);
	Array_Init(&this->mappings,  0);
	Array_Init(&this->linkpaths, 0);
}

void Builder_Destroy(Builder *this) {
	String_Destroy(&this->output);
	String_Destroy(&this->cc);
	String_Destroy(&this->inclhdr);
	String_Destroy(&this->std);

	StringArray_Destroy(&this->link);
	StringArray_Destroy(&this->linkpaths);

	Array_Destroy(&this->queue, ^(Builder_QueueItem *item) {
		String_Destroy(&item->source);
		String_Destroy(&item->output);
	});

	Array_Destroy(&this->mappings, ^(Deps_Mapping *item) {
		String_Destroy(&item->src);
		String_Destroy(&item->dest);
	});
}

bool Builder_SetOption(Builder *this, String name, String value) {
	if (String_Equals(name, $("output"))) {
		String_Copy(&this->output, value);
	} else if (String_Equals(name, $("map"))) {
		StringArray parts = String_Split(value, ':');

		if (parts.len < 2) {
			Logger_Log(&logger, Logger_Level_Error,
				$("`map' requires two values separated by a colon."));

			StringArray_Destroy(&parts);

			return false;
		}

		Deps_Mapping insert;

		insert.src  = String_Clone(parts.buf[0]);
		insert.dest = String_Clone(parts.buf[1]);

		StringArray_Destroy(&parts);

		if (insert.src.len == 0) {
			Logger_Log(&logger, Logger_Level_Error,
				$("Invalid source path."));

			String_Destroy(&insert.src);
			String_Destroy(&insert.dest);

			return false;
		}

		if (!Path_Exists(insert.dest)) {
			Logger_LogFmt(&logger, Logger_Level_Error,
				$("Destination path '%' does not exist."), insert.dest);

			String_Destroy(&insert.src);
			String_Destroy(&insert.dest);

			return false;
		}

		Array_Push(&this->mappings, insert);
	} else if (String_Equals(name, $("cc"))) {
		String_Copy(&this->cc, value);
	} else if (String_Equals(name, $("inclhdr"))) {
		String_Copy(&this->inclhdr, value);
	} else if (String_Equals(name, $("dbgsym"))) {
		this->dbgsym = String_Equals(value, $("yes"));
	} else if (String_Equals(name, $("std"))) {
		String_Copy(&this->std, value);
	} else if (String_Equals(name, $("blocks"))) {
		this->blocks = String_Equals(value, $("yes"));
	} else if (String_Equals(name, $("optimlevel"))) {
		this->optmlevel = Integer_ParseString(value);
	} else if (String_Equals(name, $("link"))) {
		StringArray_Destroy(&this->link);
		this->link = String_Split(value, ',');
	} else if (String_Equals(name, $("linkpath"))) {
		StringArray_Destroy(&this->linkpaths);
		this->linkpaths = String_Split(value, ',');
	} else if (String_Equals(name, $("verbose"))) {
		this->verbose = true;
	}

	return true;
}

String Builder_ShrinkPathEx(String shortpath, String path) {
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

String Builder_ShrinkPath(Builder *this, String path) {
	for (size_t i = 0; i < this->mappings.len; i++) {
		String shortpath = this->mappings.buf[i].src;

		String res = Builder_ShrinkPathEx(shortpath, path);

		if (res.len > 0) {
			return res;
		}
	}

	return String_Clone(path);
}

String Builder_GetOutput(Builder *this, String path) {
	String realpath = Path_Resolve(path);

	for (size_t i = 0; i < this->mappings.len; i++) {
		String mapping = Path_Resolve(this->mappings.buf[i].src);

		if (String_BeginsWith(realpath, mapping)) {
			String out = String_Clone(this->mappings.buf[i].dest);
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

String Builder_GetSource(String path) {
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

void Builder_AddToQueue(Builder *this, String source, String output) {
	for (size_t i = 0; i < this->queue.len; i++) {
		if (String_Equals(this->queue.buf[i].source, source)) {
			return;
		}
	}

	Builder_QueueItem item;
	item.source = String_Clone(source);
	item.output = String_Clone(output);

	Array_Push(&this->queue, item);
}

bool Builder_Compile(Builder *this, String src, String out) {
	Process proc;
	Process_Init(&proc, this->cc);

	Process_AddParameter(&proc, $("-o"));
	Process_AddParameter(&proc, out);

	Process_AddParameter(&proc, $("-c"));
	Process_AddParameter(&proc, src);

	String tmp;
	Process_AddParameter(&proc, tmp = String_Format($("-std=%"), this->std));
	String_Destroy(&tmp);

	if (this->blocks) {
		Process_AddParameter(&proc, $("-fblocks"));
	}

	String optim = String_Format($("-O%"), Integer_ToString(this->optmlevel));
	Process_AddParameter(&proc, optim);
	String_Destroy(&optim);

	if (this->dbgsym) {
		Process_AddParameter(&proc, $("-g"));
	}

	Process_AddParameter(&proc, $("-W"));
	Process_AddParameter(&proc, $("-Wall"));
	Process_AddParameter(&proc, $("-pipe"));

	if (this->inclhdr.len > 0) {
		Process_AddParameter(&proc, $("-include"));
		Process_AddParameter(&proc, this->inclhdr);
	}

	for (size_t i = 0; i < this->deps->include.len; i++) {
		Process_AddParameter(&proc, $("-I"));
		Process_AddParameter(&proc, this->deps->include.buf[i]);
	}

	int res = Process_Spawn(&proc);

	if (this->verbose) {
		String cmd = Process_GetCommandLine(&proc);
		Logger_Log(&logger, Logger_Level_Info, cmd);
		String_Destroy(&cmd);
	}

	Process_Destroy(&proc);

	return res < 0;
}

void Builder_Link(Builder *this, StringArray files) {
	Process proc;
	Process_Init(&proc, this->cc);

	Process_AddParameter(&proc, $("-o"));
	Process_AddParameter(&proc, this->output);

	for (size_t i = 0; i < files.len; i++) {
		Process_AddParameter(&proc, files.buf[i]);
	}

	for (size_t i = 0; i < this->linkpaths.len; i++) {
		Process_AddParameter(&proc, $("-L"));
		Process_AddParameter(&proc, this->linkpaths.buf[i]);
	}

	if (this->blocks) {
		Process_AddParameter(&proc, $("-lBlocksRuntime"));
	}

	if (this->dbgsym) {
		Process_AddParameter(&proc, $("-g"));
	}

	for (size_t i = 0; i < this->link.len; i++) {
		if (this->link.buf[i].len == 0) {
			continue;
		}

		if (this->link.buf[i].buf[0] == '@') {
			Process_AddParameter(&proc, $("-Wl,-Bdynamic"));
		} else {
			Process_AddParameter(&proc, $("-Wl,-Bstatic"));
		}

		Process_AddParameter(&proc, $("-l"));
		Process_AddParameter(&proc, String_FastSlice(
			this->link.buf[i],
			this->link.buf[i].buf[0] == '@'));
	}

	Process_Spawn(&proc);

	if (this->verbose) {
		String cmd = Process_GetCommandLine(&proc);
		Logger_Log(&logger, Logger_Level_Info, cmd);
		String_Destroy(&cmd);
	}

	Process_Destroy(&proc);
}

bool Builder_CreateQueue(Builder *this) {
	for (size_t i = 0; i < this->deps->deps.len; i++) {
		Deps_Node *dep = this->deps->deps.buf[i];

		String headerPath = String_Clone(dep->path);
		String sourcePath = Builder_GetSource(headerPath);

		/* Skip all non-source files. */
		if (sourcePath.len == 0) {
			Logger_LogFmt(&logger,
				Logger_Level_Debug,
				$("'%' has no corresponding source file"),
				headerPath);

			String_Destroy(&headerPath);
			String_Destroy(&sourcePath);

			continue;
		}

		Logger_LogFmt(&logger, Logger_Level_Info, $("Processing %..."), sourcePath);

		String output = Builder_GetOutput(this, sourcePath);

		if (output.len == 0) {
			Logger_LogFmt(&logger,
				Logger_Level_Error,
				$("No output path for '%' is mapped."),
				sourcePath);

			return false;
		}

		/* Will be true when at least one dependency has changed. */
		bool depChanged = false;

		if (dep->len > 0) {
			Logger_Log(&logger, Logger_Level_Debug, $("Depends on:"));

			for (size_t j = 0; j < dep->len; j++) {
				Logger_LogFmt(&logger, Logger_Level_Debug, $(" - %"), dep->nodes[j]->path);

				String depHeaderPath = String_Clone(dep->nodes[j]->path);
				String depSourcePath = Builder_GetSource(dep->nodes[j]->path);

				if (depSourcePath.len == 0) { /* Header file wihout matching source file. */
					if (Path_Exists(output)) {
						if (File_IsModified(depHeaderPath, output)) {
							Logger_Log(&logger, Logger_Level_Info, $("dep header changed."));
							depChanged = true;
						}
					}
				} else { /* There is a source file. */
					String depOutput = Builder_GetOutput(this, depSourcePath);

					if (depOutput.len == 0) {
						Logger_LogFmt(&logger,
							Logger_Level_Error,
							$("No output path for '%' is mapped."),
							depSourcePath);

						return false;
					}

					if (!Path_Exists(depOutput)) {
						/* dep unbuilt */
						Builder_AddToQueue(this, depSourcePath, depOutput);
						depChanged = true;
					} else if (File_IsModified(depSourcePath, depOutput)) {
						/* dep source changed */
						Builder_AddToQueue(this, depSourcePath, depOutput);
						depChanged = true;
					} else if (File_IsModified(depHeaderPath, depOutput)) {
						/* dep header changed */
						Builder_AddToQueue(this, depSourcePath, depOutput);
						depChanged = true;
					}

					String_Destroy(&depOutput);
				}

				String_Destroy(&depSourcePath);
				String_Destroy(&depHeaderPath);
			}
		}

		if (depChanged) {
			Logger_Log(&logger, Logger_Level_Info, $("Dependency changed or unbuilt."));
			Builder_AddToQueue(this, sourcePath, output);
		} else if (!Path_Exists(output)) {
			Logger_Log(&logger, Logger_Level_Info, $("Not built yet."));
			Builder_AddToQueue(this, sourcePath, output);
		} else if (File_IsModified(sourcePath, output)) {
			Logger_Log(&logger, Logger_Level_Info, $("Source modified."));
			Builder_AddToQueue(this, sourcePath, output);
		} else if (File_IsModified(headerPath, output)) {
			Logger_Log(&logger, Logger_Level_Info, $("Header modified."));
			Builder_AddToQueue(this, sourcePath, output);
		} else {
			Logger_Log(&logger, Logger_Level_Debug, $("Already built."));
		}

		String_Destroy(&sourcePath);
		String_Destroy(&headerPath);
		String_Destroy(&output);
	}

	return true;
}

void Builder_PrintQueue(Builder *this) {
	if (this->queue.len == 0 && Path_Exists(this->output)) {
		Logger_Log(&logger, Logger_Level_Info, $("  Queue is empty."));
	} else {
		Logger_Log(&logger, Logger_Level_Info, $("  Queue:"));

		for (size_t i = 0; i < this->queue.len; i++) {
			Logger_LogFmt(&logger, Logger_Level_Info, $(" - % --> %"),
				this->queue.buf[i].source,
				this->queue.buf[i].output);
		}

		Logger_LogFmt(&logger, Logger_Level_Info, $(" - % (link)"), this->output);
	}
}

bool Builder_Run(Builder *this) {
	if (this->queue.len != 0 || !Path_Exists(this->output)) {
		for (size_t i = 0; i < this->queue.len; i++) {
			String create = Path_GetDirectory(this->queue.buf[i].output);
			if (!Path_Exists(create)) {
				Path_Create(create, true);
			}
			String_Destroy(&create);

			String path = Builder_ShrinkPath(this, this->queue.buf[i].source);

			Logger_LogFmt(&logger, Logger_Level_Info, $("Compiling %... [%/%]"),
				path,
				Integer_ToString(i + 1),
				Integer_ToString(this->queue.len));

			bool ok = Builder_Compile(this, path, this->queue.buf[i].output);

			String_Destroy(&path);

			if (!ok) {
				return false;
			}
		}

		StringArray files;
		Array_Init(&files, 0);

		for (size_t i = 0; i < this->deps->deps.len; i++) {
			String src = Builder_GetSource(this->deps->deps.buf[i]->path);

			if (src.len > 0) {
				String path = Builder_GetOutput(this, src);

				if (path.len == 0) {
					StringArray_Destroy(&files);
					String_Destroy(&src);
					return false;
				}

				Array_Push(&files, Builder_ShrinkPath(this, path));

				String_Destroy(&path);
			}

			String_Destroy(&src);
		}

		Builder_Link(this, files);

		StringArray_Destroy(&files);
	}

	return true;
}
