#import "Deps.h"
#import <App.h>

extern Logger logger;

def(void, Init) {
	this->main = HeapString(0);

	Array_Init(this->deps,    0);
	Array_Init(this->include, 0);

	Tree_Init(&this->tree, (void *) &ref(DestroyNode));

	this->node = (ref(Node) *) &this->tree.root;
}

def(void, Destroy) {
	Tree_Destroy(&this->tree);
	String_Destroy(&this->main);

	foreach (item, this->include) {
		String_Destroy(item);
	}

	Array_Destroy(this->include);

	Array_Destroy(this->deps);
}

void ref(DestroyNode)(ref(Node) *node) {
	String_Destroy(&node->path);
	String_Destroy(&node->module);
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
		Array_Push(this->include, String_Clone(value));
	}

	return true;
}

def(StringArray *, GetIncludes) {
	return this->include;
}

def(DepsArray *, GetDeps) {
	return this->deps;
}

static def(String, GetLocalPath, String base, String file) {
	String path = String_Format($("%/%"), base, file);

	if (!Path_Exists(path)) {
		path.len = 0;
	}

	return path;
}

/* Iterates over all include paths and uses the matching one. */
static def(String, GetSystemPath, String file) {
	String path = HeapString(0);

	for (size_t i = 0; i < this->include->len; i++) {
		String_Append(&path, this->include->buf[i]);
		String_Append(&path, '/');
		String_Append(&path, file);

		if (Path_Exists(path)) {
			return path;
		}

		path.len = 0;
	}

	return path;
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

static def(ref(Node) *, AddFile, String absPath) {
	this->node = Tree_AddNode(this->node);

	this->node->path   = String_Clone(absPath);
	this->node->module = HeapString(0);

	for (size_t i = 0; i < this->deps->len; i++) {
		if (String_Equals(this->deps->buf[i]->path, absPath)) {
			/* Copy the module name from the first scan. Otherwise
			 * we would have to read the file a second time.
			 */
			String_Copy(&this->node->module,
				this->deps->buf[i]->module);

			return NULL;
		}
	}

	Array_Push(this->deps, this->node);
	return this->node;
}

static def(void, ScanFile, ref(Node) *node, String path) {
	String s = HeapString(1024 * 15);
	File_GetContents(path, &s);
	StringArray *lines = String_Split(s, '\n');

	for (size_t i = 0; i < lines->len; i++) {
		String needle;

		if (String_BeginsWith(lines->buf[i], needle = $("#define self "))) {
			String_Copy(&node->module,
				String_Trim(
					String_Slice(
						lines->buf[i],
						needle.len)));

			continue;
		}

		if (!String_BeginsWith(lines->buf[i], needle = $("#include "))
		 && !String_BeginsWith(lines->buf[i], needle = $("#import "))) {
			continue;
		}

		String type =
			String_Trim(
				String_Slice(
					lines->buf[i],
					needle.len));

		String header;
		bool quotes = false;

		if (type.buf[0] == '<') {
			quotes = false;
			header =
				String_Trim(
					String_Between(
						lines->buf[i],
						$("<") ,
						$(">")));
		} else if (type.buf[0] == '"') {
			quotes = true;
			header =
				String_Trim(
					String_Between(
						lines->buf[i],
						$("\""),
						$("\"")));
		} else {
			Logger_Error(&logger, $("Line '%' not understood."),
				lines->buf[i]);

			continue;
		}

		ref(Type) deptype = quotes
			? ref(Type_Local)
			: ref(Type_System);

		call(ProcessFile,
			Path_GetDirectory(path),
			header, deptype);
	}

	Array_Destroy(lines);
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
			ref(Node) *node = call(AddFile, absPath);

			if (node != NULL) {
				Logger_Debug(&logger, $("Adding '%'..."), absPath);
				call(ScanFile, node, absPath);
			}

			this->node = this->node->parent;
		}

		String_Destroy(&absPath);
	}

	String_Destroy(&relPath);
}

def(void, ListSourceFiles) {
	for (size_t i = 0; i < this->deps->len; i++) {
		String path = String_Clone(this->deps->buf[i]->path);

		path.buf[path.len - 1] = 'c';

		if (Path_Exists(path)) {
			Logger_Debug(&logger, path);
		}

		String_Destroy(&path);
	}
}

static def(void, PrintNode, ref(Node) *node, int indent) {
	if (node == (ref(Node) *) &this->tree.root) {
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

	if (node->module.len > 0) {
		String_Print($(" ("));
		String_Print(node->module);
		String_Print($(")"));
	}

	String_Print($("]\n"));

iter:
	for (size_t i = 0; i < node->len; i++) {
		call(PrintNode, node->buf[i], indent + 2);
	}
}

def(void, PrintTree) {
	call(PrintNode, this->node, 0);
}

def(void, Scan) {
	if (this->main.len > 0 && !Path_Exists(this->main)) {
		Logger_Error(&logger, $("Main file '%' not found."),
			this->main);

		return;
	}

	call(ProcessFile, $("."), this->main, ref(Type_Local));
}
