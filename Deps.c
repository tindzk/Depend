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

	Array_Foreach(this->include, String_Destroy);
	Array_Destroy(this->include);

	Array_Destroy(this->deps);
}

void ref(DestroyNode)(ref(Node) *node) {
	String_Destroy(&node->path);
}

def(bool, SetOption, String name, String value) {
	if (String_Equals(name, $("main"))) {
		String_Copy(&this->main, value);
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
		String_Destroy(&path);
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

static def(bool, AddFile, String absPath) {
	bool alreadyExistent = false;

	for (size_t i = 0; i < this->deps->len; i++) {
		if (String_Equals(this->deps->buf[i]->path, absPath)) {
			alreadyExistent = true;
			break;
		}
	}

	this->node = Tree_AddNode(this->node);

	this->node->path = String_Clone(absPath);

	if (!alreadyExistent) {
		Array_Push(this->deps, this->node);
	}

	return alreadyExistent;
}

static def(void, ScanFile, String file);

static def(void, ScanFileDeps, String base, StringArray *arr) {
	for (size_t i = 0; i < arr->len; i++) {
		String tmp;

		if (!String_BeginsWith(arr->buf[i], tmp = $("#include"))
		 && !String_BeginsWith(arr->buf[i], tmp = $("#import"))) {
			continue;
		}

		if (arr->buf[i].len < tmp.len + 1) {
			continue;
		}

		String type =
			String_Clone(
				String_Trim(
					String_Slice(arr->buf[i], tmp.len + 1)));

		String header;
		bool quotes = false;

		if (type.buf[0] == '<') {
			quotes = false;
			header =
				String_Trim(
					String_Between(
						arr->buf[i],
						$("<") ,
						$(">")));
		} else if (type.buf[0] == '"') {
			quotes = true;
			header =
				String_Trim(
					String_Between(
						arr->buf[i],
						$("\""),
						$("\"")));
		} else {
			Logger_LogFmt(&logger,
				Logger_Level_Error,
				$("Line '%' not understood."),
				arr->buf[i]);

			continue;
		}

		String_Destroy(&type);

		Logger_LogFmt(&logger, Logger_Level_Debug,
			$("Header file '%' found."), header);

		ref(Type) deptype = quotes
			? ref(Type_Local)
			: ref(Type_System);

		String relPath = call(GetFullPath, base, header, deptype);

		if (relPath.len > 0) {
			String absPath = Path_Resolve(relPath);

			if (absPath.len > 0) {
				bool scanned = call(AddFile, absPath);

				if (!scanned) {
					call(ScanFile, absPath);
				}

				this->node = this->node->parent;
			}

			String_Destroy(&absPath);
		}

		String_Destroy(&relPath);
		String_Destroy(&header);
	}
}

static def(void, ScanFile, String file) {
	Logger_LogFmt(&logger, Logger_Level_Debug, $("Adding '%'..."), file);

	String s = HeapString(1024 * 15);

	File_GetContents(file, &s);

	StringArray *arr = String_Split(s, '\n');

	String base = Path_GetDirectory(file);

	call(ScanFileDeps, base, arr);

	Array_Destroy(arr);
	String_Destroy(&s);
}

def(void, ListSourceFiles) {
	for (size_t i = 0; i < this->deps->len; i++) {
		String path = String_Clone(this->deps->buf[i]->path);

		path.buf[path.len - 1] = 'c';

		if (Path_Exists(path)) {
			Logger_Log(&logger, Logger_Level_Debug, path);
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
	String_Print($("]\n"));

iter:
	for (size_t i = 0; i < node->len; i++) {
		call(PrintNode, node->nodes[i], indent + 2);
	}
}

def(void, PrintTree) {
	call(PrintNode, this->node, 0);
}

def(void, Scan) {
	String fullpath = Path_Resolve(this->main);

	if (fullpath.len == 0 || !Path_Exists(fullpath)) {
		Logger_LogFmt(&logger,
			Logger_Level_Error,
			$("Main file '%' not found."),
			this->main);
	} else {
		call(AddFile,  fullpath);
		call(ScanFile, fullpath);

		this->node = this->node->parent;
	}

	String_Destroy(&fullpath);
}
