#import "Deps.h"

#define self Deps

def(void, Init, Logger *logger) {
	this->logger  = logger;
	this->main    = String_New(0);
	this->modules = scall(Modules_New, 0);
	this->include = StringArray_New(0);
	this->paths   = StringArray_New(0);
	this->tree    = Tree_New(Tree_DestroyNode_For(this, ref(DestroyNode)));

	this->component = (ref(Component) *) &this->tree.root;
}

def(void, Destroy) {
	Tree_Destroy(&this->tree);
	String_Destroy(&this->main);

	StringArray_Destroy(this->paths);
	StringArray_Free(this->paths);

	StringArray_Destroy(this->include);
	StringArray_Free(this->include);

	each(module, this->modules) {
		String_Destroy(&module->name);

		StringArray_Destroy(module->exc);
		StringArray_Free(module->exc);
	}

	scall(Modules_Free, this->modules);
}

def(void, DestroyNode, Tree_Node *ptr) {
	ref(Component) *node = (ref(Component) *) ptr;

	String_Destroy(&node->path);
	scall(ModuleOffsets_Free, node->modules);
}

static def(void, ProcessFile, RdString base, RdString file, ref(Type) deptype);

/* Very simple glob() implementation. Only one placeholder is
 * allowed. Escaping and expanding (e.g. {a, b, c}) might be
 * implemented in the future. If so, this function should be
 * moved to the Jivai sources.
 */
static def(void, Add, RdString value) {
	ssize_t star = String_Find(value, '*');

	if (star == String_NotFound) {
		if (value.len > 0 && !Path_Exists(value)) {
			Logger_Error(this->logger,
				$("Manually added file '%' not found."),
				value);
		} else {
			call(ProcessFile, $("."), value, ref(Type_Local));
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
		if (item.type != Directory_ItemType_Symlink
		 && item.type != Directory_ItemType_Regular) {
			continue;
		}

		if (String_BeginsWith(item.name, left) &&
			String_EndsWith(item.name, right))
		{
			call(ProcessFile, path, item.name, ref(Type_Local));
		}
	}

	Directory_Destroy(&dir);
}

def(bool, SetOption, RdString name, RdString value) {
	if (String_Equals(name, $("main"))) {
		String_Copy(&this->main, value);
	} else if (String_Equals(name, $("add"))) {
		call(Add, value);
	} else if (String_Equals(name, $("include"))) {
		StringArray_Push(&this->include, String_Clone(value));
	}

	return true;
}

def(StringArray *, GetIncludes) {
	return this->include;
}

def(ref(Modules) *, GetModules) {
	return this->modules;
}

def(ref(Component) *, GetComponent) {
	return this->component;
}

def(StringArray *, GetPaths) {
	return this->paths;
}

static def(String, GetLocalPath, RdString base, RdString file) {
	String path = String_Format($("%/%"), base, file);

	if (Path_Exists(path.rd)) {
		return path;
	}

	String_Destroy(&path);
	return String_New(0);
}

/* Iterates over all include paths and uses the matching one. */
static def(String, GetSystemPath, RdString file) {
	String path = String_New(0);

	fwd(i, this->include->len) {
		path.len = 0;

		String_Append(&path, this->include->buf[i].rd);
		String_Append(&path, '/');
		String_Append(&path, file);

		if (Path_Exists(path.rd)) {
			return path;
		}
	}

	String_Destroy(&path);
	return String_New(0);
}

static def(String, GetFullPath, RdString base, RdString file, ref(Type) type) {
	String path;

	if (type == ref(Type_Local)) {
		path = call(GetLocalPath, base, file);

		if (path.len == 0) {
			path = call(GetSystemPath, file);
		}
	} else {
		path = call(GetSystemPath, file);

		if (path.len == 0) {
			path = call(GetLocalPath, base, file);
		}
	}

	return path;
}

static def(ref(Component) *, AddFile, RdString absPath) {
	this->component = Tree_AddNode(&this->tree, this->component);

	this->component->path    = String_Clone(absPath);
	this->component->modules = scall(ModuleOffsets_New, 1);

	if (StringArray_Contains(this->paths, absPath)) {
		return NULL;
	}

	StringArray_Push(&this->paths, String_Clone(absPath));

	return this->component;
}

static def(void, ScanFile, RdString path) {
	String s = String_New(1024 * 15);
	File_GetContents(path, &s);

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

			scall(ModuleOffsets_Push, &this->component->modules, ofsModule);

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

		header = String_Trim(header);

		ref(Type) deptype = quotes
			? ref(Type_Local)
			: ref(Type_System);

		call(ProcessFile,
			Path_GetDirectory(path),
			header, deptype);
	}

	String_Destroy(&s);
}

static def(void, ProcessFile, RdString base, RdString file, ref(Type) deptype) {
	if (file.len == 0) {
		return;
	}

	String relPath = call(GetFullPath, base, file, deptype);

	if (relPath.len > 0) {
		String absPath = Path_Resolve(relPath.rd);

		if (absPath.len > 0) {
			/* Returns a pointer to the node when the file wasn't
			 * scanned yet.
			 */
			ref(Component) *node = call(AddFile, absPath.rd);

			if (node != NULL) {
				Logger_Debug(this->logger, $("Adding '%'..."), absPath.rd);
				call(ScanFile, absPath.rd);
			}

			this->component = this->component->parent;
		}

		String_Destroy(&absPath);
	}

	String_Destroy(&relPath);
}

def(void, ListSourceFiles) {
	fwd(i, this->paths->len) {
		String path = String_Clone(this->paths->buf[i].rd);

		path.buf[path.len - 1] = 'c';

		if (Path_Exists(path.rd)) {
			Logger_Debug(this->logger, path.rd);
		}

		String_Destroy(&path);
	}
}

static def(void, PrintNode, ref(Component) *node, int indent) {
	if (node == (ref(Component) *) &this->tree.root) {
		/* The root node does not contain a path. */
		goto iter;
	}

	for (int in = 0; in < indent - 2; in++) {
		String_Print($(" "));
	}

	if (indent >= 2) {
		String_Print($("\\"));
	}

	if (indent >= 1) {
		String_Print($("="));
	}

	String_Print($("["));
	String_Print(node->path.rd);

	if (node->modules->len > 0) {
		String_Print($(" ("));

		each(module, node->modules) {
			String_Print(this->modules->buf[*module].name.rd);

			if (!isLast(module, node->modules)) {
				String_Print($(", "));
			}
		}

		String_Print($(")"));
	}

	String_Print($("]\n"));

iter:
	for (size_t i = 0; i < node->len; i++) {
		call(PrintNode, node->buf[i], indent + 2);
	}
}

def(void, PrintTree) {
	call(PrintNode, this->component, 0);
}

def(void, Scan) {
	if (this->main.len > 0 && !Path_Exists(this->main.rd)) {
		Logger_Error(this->logger, $("Main file '%' not found."),
			this->main.rd);

		return;
	}

	call(ProcessFile, $("."), this->main.rd, ref(Type_Local));
}
