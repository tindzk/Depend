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
		String_Destroy(&component->source);
		String_Destroy(&component->header);
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
	String_Append(path, base);
	String_Append(path, file);

	if (Path_exists(path->rd)) {
		return true;
	}

	path->len = 0;

	return false;
}

/* Iterates over all include paths and uses the matching one. */
static def(bool, getSystemPath, RdString file, String *path) {
	fwd(i, this->include->len) {
		String_Append(path, this->include->buf[i].rd);
		String_Append(path, file);

		if (Path_exists(path->rd)) {
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
		if (String_Equals(this->components->buf[i].header.rd, absPath) ||
			String_Equals(this->components->buf[i].source.rd, absPath))
		{
			return i;
		}
	}

	return -1;
}

static def(size_t, addComponent, String absPath, bool source) {
	String hdr, src;

	if (source) {
		src = absPath;
		hdr = String_Format($("%h"), String_Slice(absPath.rd, 0, -1));

		if (!Path_exists(hdr.rd)) {
			Logger_Debug(this->logger,
				t("'%' has no corresponding header file."), src.rd);
			hdr.len = 0;
		}
	} else {
		src = String_Format($("%c"), String_Slice(absPath.rd, 0, -1));
		hdr = absPath;

		if (!Path_exists(src.rd)) {
			Logger_Debug(this->logger,
				t("'%' has no corresponding source file."), hdr.rd);
			src.len = 0;
		}
	}

	ref(Component) comp = {
		.source  = src,
		.header  = hdr,
		.modules = scall(ModuleOffsets_New, 1),
		.deps    = scall(ComponentOffsets_New, 16),
		.build   = false
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
		 absPath = Path_expand(relPath.rd);
	}

	String_Destroy(&relPath);

	return absPath;
}

static def(void, scanFile, size_t ofs) {
	ref(Component) *comp = &this->components->buf[ofs];

	RdString path = (comp->header.len != 0)
		? comp->header.rd
		: comp->source.rd;

	String s = String_New(1024 * 15);
	File_getContents(path, &s);

	ssize_t ofsModule = -1;

	RdString iter = $("");
	while (String_Split(s.rd, '\n', &iter)) {
		RdString line = String_Trim(iter);

		RdString name;
		if (String_Parse($("exc(%)"), line, &name)) {
			name = String_Trim(name);

			if (ofsModule == -1) {
				Logger_Error(this->logger, t("Ignored exception '%'."), name);
			} else {
				Logger_Debug(this->logger, t("Found exception %."), name);
				StringArray_Push(&this->modules->buf[ofsModule].exc, String_Clone(name));
			}

			continue;
		}

		RdString needle;
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
				Logger_Debug(this->logger, t("Found module %."), name);
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

		bool quotes = false;
		RdString header = $("");

		if (type.buf[0] == '<') {
			quotes = false;
			String_Between(line, $("<"), $(">"), &header);
		} else if (type.buf[0] == '"') {
			quotes = true;
			String_Between(line, $("\""), $("\""), &header);
		} else {
			Logger_Error(this->logger, t("Line '%' not understood."), line);
			continue;
		}

		RdString dep = String_Trim(header);

		ref(Type) deptype = quotes
			? ref(Type_Local)
			: ref(Type_System);

		Logger_Debug(this->logger, t("Resolving dependency '%'..."), dep);

		String absPath = call(resolve, Path_getFolderPath(path), dep, deptype);

		if (absPath.len == 0) {
			Logger_Debug(this->logger, t("Dependency not found."));
		} else {
			Logger_Debug(this->logger, t("Absolute path is '%'."), absPath.rd);

			/* Dependency offset. */
			ssize_t depOfs = call(getComponentOffset, absPath.rd);

			if (depOfs == -1) {
				depOfs = call(addComponent, absPath, false);
				Logger_Debug(this->logger, t("Scanning dependency..."));
				call(scanFile, depOfs);
			} else {
				String_Destroy(&absPath);
			}

			/* We must assume that the calls to resolve() and
			 * addComponent() have pushed some elements to the array
			 * which may have caused the pointer to be invalid now.
			 */
			comp = &this->components->buf[ofs];
			scall(ComponentOffsets_Push, &comp->deps, depOfs);
		}
	}

	String_Destroy(&s);
}

static def(void, processSourceFile, RdString base, RdString file, ref(Type) deptype) {
	String absPath = call(resolve, base, file, deptype);

	if (absPath.len != 0 && call(getComponentOffset, absPath.rd) == -1) {
		Logger_Debug(this->logger, t("Adding '%'..."), absPath.rd);

		size_t pos = call(addComponent, absPath, true);
		call(scanFile, pos);
	} else {
		String_Destroy(&absPath);
	}
}

static def(void, onMatch, RdString path, RdString name) {
	call(processSourceFile, path, name, ref(Type_Local));
}

def(void, add, RdString pattern) {
	Folder_Expander expander = Folder_Expander_new(
		Folder_Expander_OnMatch_For(this, ref(onMatch)));

	bool isPattern = Folder_Expander_scan(&expander, pattern);

	if (!isPattern) {
		if (pattern.len > 0 && !Path_exists(pattern)) {
			Logger_Error(this->logger,
				t("Manually added file '%' not found."),
				pattern);
		} else {
			call(processSourceFile, $("./"), pattern, ref(Type_Local));
		}
	}
}

def(void, setMain, RdString value) {
	String_Copy(&this->main, value);
}

def(RdString, getMain) {
	return this->main.rd;
}

def(void, addInclude, RdString value) {
	StringArray_Push(&this->include, String_Clone(value));
}

def(void, scan, RdString basePath) {
	if (this->main.len == 0) {
		RdString folderName = Path_getFolderName(basePath);

		if (folderName.len == 0) {
			Logger_Error(this->logger, t("No main file set."));
			return;
		}

		String_Append(&this->main, folderName);
		String_Append(&this->main, $(".c"));
	}

	if (!Path_exists(this->main.rd)) {
		Logger_Error(this->logger, t("Main file '%' not found."),
			this->main.rd);
		return;
	}

	fwd(i, this->include->len) {
		if (!Path_isFolderPath(this->include->buf[i].rd) ||
			!Path_exists(this->include->buf[i].rd))
		{
			Logger_Error(this->logger,
				t("Include path '%' is invalid. Trailing slash missing?"),
				this->include->buf[i].rd);
			return;
		}
	}

	call(processSourceFile, $("./"), this->main.rd, ref(Type_Local));
}
