#import "Deps.h"

#define self Deps

extern Logger logger;

def(void, Init) {
	this->main    = $("");
	this->modules = scall(Modules_New, 0);
	this->include = StringArray_New(0);
	this->paths   = StringArray_New(0);

	Tree_Init(&this->tree, (void *) ref(DestroyNode));

	this->component = (ref(Component) *) &this->tree.root;
}

def(void, Destroy) {
	Tree_Destroy(&this->tree);
	String_Destroy(&this->main);

	StringArray_Destroy(this->paths);
	StringArray_Free(this->paths);

	StringArray_Destroy(this->include);
	StringArray_Free(this->include);

	foreach (module, this->modules) {
		String_Destroy(&module->name);

		StringArray_Destroy(module->exc);
		StringArray_Free(module->exc);
	}

	scall(Modules_Free, this->modules);
}

sdef(void, DestroyNode, ref(Component) *node) {
	String_Destroy(&node->path);
	scall(ModuleOffsets_Free, node->modules);
}

static def(void, ProcessFile, String base, String file, ref(Type) deptype);

/* Very simple glob() implementation. Only one placeholder is
 * allowed. Escaping and expanding (e.g. {a, b, c}) might be
 * implemented in the future. If so, this function should be
 * moved to the Jivai sources.
 */
static def(void, Add, String value) {
	ssize_t star = String_Find(value, '*');

	if (star == String_NotFound) {
		if (value.len > 0 && !Path_Exists(value)) {
			Logger_Error(&logger,
				$("Manually added file '%' not found."),
				value);
		} else {
			call(ProcessFile, $("."), value, ref(Type_Local));
		}

		return;
	}

	ssize_t slash = String_ReverseFind(value, star, '/');

	String path, left;

	if (slash == String_NotFound) {
		path = $(".");
		left = String_Slice(value, 0, star);
	} else {
		path = String_Slice(value, 0, slash);
		left = String_Slice(value, slash + 1, star - slash - 1);
	}

	String right = String_Slice(value, star + 1);

	Directory dir;
	Directory_Entry item;
	Directory_Init(&dir, path);

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

def(bool, SetOption, String name, String value) {
	if (String_Equals(name, $("main"))) {
		String_Copy(&this->main, value);
	} else if (String_Equals(name, $("add"))) {
		call(Add, value);
	} else if (String_Equals(name, $("include"))) {
		String *push = New(String);
		*push = String_Clone(value);

		StringArray_Push(&this->include, push);
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

static def(String, GetLocalPath, String base, String file) {
	String path = String_Format($("%/%"), base, file);

	if (Path_Exists(path)) {
		return path;
	}

	String_Destroy(&path);
	return $("");
}

/* Iterates over all include paths and uses the matching one. */
static def(String, GetSystemPath, String file) {
	String path = $("");

	forward (i, this->include->len) {
		path.len = 0;

		String_Append(&path, *this->include->buf[i]);
		String_Append(&path, '/');
		String_Append(&path, file);

		if (Path_Exists(path)) {
			return path;
		}
	}

	String_Destroy(&path);
	return $("");
}

static def(String, GetFullPath, String base, String file, ref(Type) type) {
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

static def(ref(Component) *, AddFile, String absPath) {
	this->component = Tree_AddNode(this->component);

	this->component->path    = String_Clone(absPath);
	this->component->modules = scall(ModuleOffsets_New, 1);

	if (StringArray_Contains(this->paths, absPath)) {
		return NULL;
	}

	String *push = New(String);
	*push = String_Clone(absPath);

	StringArray_Push(&this->paths, push);

	return this->component;
}

static def(void, ScanFile, String path) {
	String s = String_New(1024 * 15);
	File_GetContents(path, &s);
	StringArray *lines = String_Split(&s, '\n');

	ssize_t ofsModule = -1;

	forward (i, lines->len) {
		String needle;

		ssize_t offset = String_Find(*lines->buf[i], needle = $("@exc "));

		if (offset != String_NotFound) {
			String name =
				String_Trim(
					String_Slice(
						*lines->buf[i],
						offset + needle.len));

			if (ofsModule == -1) {
				Logger_Error(&logger, $("Ignored exception '%'."), name);
			} else {
				Logger_Debug(&logger, $("Found exception %."), name);

				String *push = New(String);
				*push = String_Clone(name);

				StringArray_Push(&this->modules->buf[ofsModule].exc, push);
			}

			continue;
		}

		if (String_BeginsWith(*lines->buf[i], needle = $("#define self "))) {
			String name =
				String_Trim(
					String_Slice(
						*lines->buf[i],
						needle.len));

			foreach (item, this->modules) {
				if (String_Equals(item->name, name)) {
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

				Logger_Debug(&logger, $("Found module %."), name);
			}

			scall(ModuleOffsets_Push, &this->component->modules, ofsModule);

			continue;
		}

		if (!String_BeginsWith(*lines->buf[i], needle = $("#include ")) &&
			!String_BeginsWith(*lines->buf[i], needle = $("#import ")))
		{
			continue;
		}

		String type =
			String_Trim(
				String_Slice(
					*lines->buf[i],
					needle.len));

		String header;
		bool quotes = false;

		if (type.buf[0] == '<') {
			quotes = false;
			header =
				String_Trim(
					String_Between(
						*lines->buf[i],
						$("<") ,
						$(">")));
		} else if (type.buf[0] == '"') {
			quotes = true;
			header =
				String_Trim(
					String_Between(
						*lines->buf[i],
						$("\""),
						$("\"")));
		} else {
			Logger_Error(&logger, $("Line '%' not understood."),
				*lines->buf[i]);

			continue;
		}

		ref(Type) deptype = quotes
			? ref(Type_Local)
			: ref(Type_System);

		call(ProcessFile,
			Path_GetDirectory(path),
			header, deptype);
	}

	StringArray_Free(lines);
	String_Destroy(&s);
}

static def(void, ProcessFile, String base, String file, ref(Type) deptype) {
	if (file.len == 0) {
		return;
	}

	String relPath = call(GetFullPath, base, file, deptype);

	if (relPath.len > 0) {
		String absPath = Path_Resolve(relPath);

		if (absPath.len > 0) {
			/* Returns a pointer to the node when the file wasn't
			 * scanned yet.
			 */
			ref(Component) *node = call(AddFile, absPath);

			if (node != NULL) {
				Logger_Debug(&logger, $("Adding '%'..."), absPath);
				call(ScanFile, absPath);
			}

			this->component = this->component->parent;
		}

		String_Destroy(&absPath);
	}

	String_Destroy(&relPath);
}

def(void, ListSourceFiles) {
	forward (i, this->paths->len) {
		String path = String_Clone(*this->paths->buf[i]);

		path.buf[path.len - 1] = 'c';

		if (Path_Exists(path)) {
			Logger_Debug(&logger, path);
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
	String_Print(node->path);

	if (node->modules->len > 0) {
		String_Print($(" ("));

		foreach (module, node->modules) {
			String_Print(this->modules->buf[*module].name);

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
	if (this->main.len > 0 && !Path_Exists(this->main)) {
		Logger_Error(&logger, $("Main file '%' not found."),
			this->main);

		return;
	}

	call(ProcessFile, $("."), this->main, ref(Type_Local));
}
