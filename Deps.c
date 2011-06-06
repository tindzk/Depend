#import "Deps.h"

#define self Deps

rsdef(self, new, Logger *logger) {
	return (self) {
		.main       = String_New(0),
		.logger     = logger,
		.include    = StringArray_New(0),
		.modules    = scall(Modules_New, 0),
		.components = scall(Components_New, 0)
	};
}

def(void, destroy) {
	each(component, this->components) {
		scall(ComponentOffsets_Free, component->deps);
		scall(ModuleOffsets_Free, component->modules);
		String_Destroy(&component->path);
	}

	scall(Components_Free, this->components);

	each(module, this->modules) {
		StringArray_Destroy(module->exc);
		StringArray_Free(module->exc);

		String_Destroy(&module->name);
	}

	scall(Modules_Free, this->modules);

	StringArray_Destroy(this->include);
	StringArray_Free(this->include);

	String_Destroy(&this->main);
}

static def(bool, getLocalPath, RdString base, RdString file, String *path) {
	String_Append(path, FmtString($("%/%"), base, file));

	if (Path_Exists(path->rd)) {
		return true;
	}

	path->len = 0;

	return false;
}

/* Iterates over all include paths and uses the matching one. */
static def(bool, getSystemPath, RdString file, String *path) {
	fwd(i, this->include->len) {
		String_Append(path, FmtString($("%/%"),
			this->include->buf[i].rd, file));

		if (Path_Exists(path->rd)) {
			return true;
		}

		path->len = 0;
	}

	return false;
}

static def(String, getFullPath, RdString base, RdString file, ref(Type) type) {
	String path = String_New(0);

	if (type == ref(Type_Local)) {
		if (!call(getLocalPath, base, file, &path)) {
			call(getSystemPath, file, &path);
		}
	} else {
		if (!call(getSystemPath, file, &path)) {
			call(getLocalPath, base, file, &path);
		}
	}

	return path;
}

static def(ssize_t, getComponentOffset, RdString absPath) {
	fwd(i, this->components->len) {
		if (String_Equals(this->components->buf[i].path.rd, absPath)) {
			return i;
		}
	}

	return -1;
}

static def(size_t, addComponent, String absPath) {
	ref(Component) comp = {
		.path    = absPath,
		.modules = scall(ModuleOffsets_New, 1),
		.deps    = scall(ComponentOffsets_New, 16)
	};

	scall(Components_Push, &this->components, comp);

	return this->components->len - 1;
}

static def(String, resolve, RdString base, RdString file, ref(Type) deptype) {
	if (file.len == 0) {
		return String_New(0);
	}

	String relPath = call(getFullPath, base, file, deptype);
	String absPath = String_New(0);

	if (relPath.len != 0) {
		 absPath = Path_Resolve(relPath.rd);
	}

	String_Destroy(&relPath);

	return absPath;
}

static def(void, scanFile, ref(Component) *comp) {
	String s = String_New(1024 * 15);
	File_GetContents(comp->path.rd, &s);

	ssize_t ofsModule = -1;

	RdString line = $("");
	while (String_Split(s.rd, '\n', &line)) {
		RdString needle;

		ssize_t offset = String_Find(line, needle = $("@exc "));

		if (offset != String_NotFound) {
			RdString name = String_Trim(String_Slice(line, offset + needle.len));

			if (ofsModule == -1) {
				Logger_Error(this->logger, $("Ignored exception '%'."), name);
			} else {
				Logger_Debug(this->logger, $("Found exception %."), name);
				StringArray_Push(&this->modules->buf[ofsModule].exc, String_Clone(name));
			}

			continue;
		}

		if (String_BeginsWith(line, needle = $("#define self "))) {
			RdString name = String_Trim(String_Slice(line, needle.len));

			each(item, this->modules) {
				if (String_Equals(item->name.rd, name)) {
					ofsModule = getIndex(item, this->modules);
					break;
				}
			}

			if (ofsModule == -1) {
				scall(Modules_Push, &this->modules,
					(ref(Module)) {
						.name = String_Clone(name),
						.exc  = StringArray_New(0)
					}
				);

				ofsModule = this->modules->len - 1;
				Logger_Debug(this->logger, $("Found module %."), name);
			}

			scall(ModuleOffsets_Push, &comp->modules, ofsModule);
			continue;
		}

		if (!String_BeginsWith(line, needle = $("#include ")) &&
			!String_BeginsWith(line, needle = $("#import ")))
		{
			continue;
		}

		RdString type = String_Trim(String_Slice(line, needle.len));

		RdString header;
		bool quotes = false;

		if (type.buf[0] == '<') {
			quotes = false;
			String_Between(line, $("<"), $(">"), &header);
		} else if (type.buf[0] == '"') {
			quotes = true;
			String_Between(line, $("\""), $("\""), &header);
		} else {
			Logger_Error(this->logger, $("Line '%' not understood."), line);
			continue;
		}

		RdString dep = String_Trim(header);

		ref(Type) deptype = quotes
			? ref(Type_Local)
			: ref(Type_System);

		Logger_Debug(this->logger, $("Resolving dependency '%'..."), dep);

		String absPath = call(resolve,
			Path_GetDirectory(comp->path.rd),
			dep, deptype);

		if (absPath.len == 0) {
			Logger_Debug(this->logger, $("Dependency not found."));
		} else {
			Logger_Debug(this->logger, $("Absolute path is '%'."), absPath.rd);

			/* Dependency offset. */
			ssize_t ofs = call(getComponentOffset, absPath.rd);

			if (ofs == -1) {
				ofs = call(addComponent, absPath);
				Logger_Debug(this->logger, $("Scanning dependency..."));
				call(scanFile, &this->components->buf[ofs]);
			} else {
				String_Destroy(&absPath);
			}

			scall(ComponentOffsets_Push, &comp->deps, ofs);
		}
	}

	String_Destroy(&s);
}

static def(void, processFile, RdString base, RdString file, ref(Type) deptype) {
	String absPath = call(resolve, base, file, deptype);

	if (absPath.len != 0 && call(getComponentOffset, absPath.rd) == -1) {
		size_t pos = call(addComponent, absPath);
		ref(Component) *comp = &this->components->buf[pos];

		Logger_Debug(this->logger, $("Adding '%'..."), absPath.rd);
		call(scanFile, comp);
	} else {
		String_Destroy(&absPath);
	}
}

/* Very simple glob() implementation. Only one placeholder is
 * allowed. Escaping and expanding (e.g. {a, b, c}) might be
 * implemented in the future. If so, this function should be
 * moved to the Jivai sources.
 */
static def(void, add, RdString value) {
	ssize_t star = String_Find(value, '*');

	if (star == String_NotFound) {
		if (value.len > 0 && !Path_Exists(value)) {
			Logger_Error(this->logger,
				$("Manually added file '%' not found."),
				value);
		} else {
			call(processFile, $("."), value, ref(Type_Local));
		}

		return;
	}

	ssize_t slash = String_ReverseFind(String_Slice(value, star), '/');

	RdString path, left;

	if (slash == String_NotFound) {
		path = $(".");
		left = String_Slice(value, 0, star);
	} else {
		slash += star;

		path = String_Slice(value, 0, slash);
		left = String_Slice(value, slash + 1, star - slash - 1);
	}

	RdString right = String_Slice(value, star + 1);

	Directory_Entry item;

	Directory dir = Directory_New(path);

	while (Directory_Read(&dir, &item)) {
		if (item.type != Directory_ItemType_Symlink &&
			item.type != Directory_ItemType_Regular)
		{
			continue;
		}

		if (String_BeginsWith(item.name, left) &&
			String_EndsWith(item.name, right))
		{
			call(processFile, path, item.name, ref(Type_Local));
		}
	}

	Directory_Destroy(&dir);
}

def(bool, setOption, RdString name, RdString value) {
	if (String_Equals(name, $("main"))) {
		String_Copy(&this->main, value);
	} else if (String_Equals(name, $("add"))) {
		call(add, value);
	} else if (String_Equals(name, $("include"))) {
		StringArray_Push(&this->include, String_Clone(value));
	}

	return true;
}

def(void, scan) {
	if (this->main.len == 0) {
		Logger_Error(this->logger, $("No main file set."));
		return;
	}

	if (!Path_Exists(this->main.rd)) {
		Logger_Error(this->logger, $("Main file '%' not found."),
			this->main.rd);
		return;
	}

	call(processFile, $("."), this->main.rd, ref(Type_Local));
}
