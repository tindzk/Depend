#include "Deps.h"

extern Logger logger;

void Deps_Init(Deps *this) {
	this->main = HeapString(0);
	StringArray_Init(&this->include, 0);
	Array_Init(&this->deps, 0);

	Tree_Init(&this->tree, (void *) &Deps_DestroyNode);

	this->node = (Deps_Node *) &this->tree.root;
}

void Deps_Destroy(Deps *this) {
	Tree_Destroy(&this->tree);
	String_Destroy(&this->main);
	StringArray_Destroy(&this->include);
	Array_Destroy(&this->deps, ^(UNUSED Deps_Node **item) { });
}

void Deps_DestroyNode(Deps_Node *node) {
	String_Destroy(&node->path);
}

bool Deps_SetOption(Deps *this, String name, String value) {
	if (String_Equals(&name, $("main"))) {
		String_Copy(&this->main, value);
	} else if (String_Equals(&name, $("include"))) {
		this->include = String_Split(&value, ',');
	}

	return true;
}

String Deps_GetLocalPath(UNUSED Deps *this, String base, String file) {
	String path = HeapString(0);
	String_Append(&path, base);
	String_Append(&path, '/');
	String_Append(&path, file);

	if (!Path_Exists(path)) {
		path.len = 0;
	}

	return path;
}

/* Iterates over all include paths and uses the matching one. */
String Deps_GetSystemPath(Deps *this, String file) {
	String path = HeapString(0);

	for (size_t i = 0; i < this->include.len; i++) {
		String_Append(&path, this->include.buf[i]);
		String_Append(&path, '/');
		String_Append(&path, file);

		if (Path_Exists(path)) {
			return path;
		}

		path.len = 0;
	}

	return path;
}

String Deps_GetFullPath(Deps *this, String base, String file, Deps_Type type) {
	String path;

	if (type == Deps_Type_Local) {
		path = Deps_GetLocalPath(this, base, file);

		if (path.len == 0) {
			String_Destroy(&path);
			path = Deps_GetSystemPath(this, file);
		}
	} else {
		path = Deps_GetSystemPath(this, file);

		if (path.len == 0) {
			String_Destroy(&path);
			path = Deps_GetLocalPath(this, base, file);
		}
	}

	return path;
}

bool Deps_AddFile(Deps *this, String absPath) {
	bool alreadyExistent = false;

	for (size_t i = 0; i < this->deps.len; i++) {
		if (String_Equals(&this->deps.buf[i]->path, absPath)) {
			alreadyExistent = true;
			break;
		}
	}

	this->node = Tree_AddNode(Deps_Node, (Tree_Node *) this->node);
	this->node->path = String_Clone(absPath);

	if (!alreadyExistent) {
		Array_Push(&this->deps, this->node);
	}

	return alreadyExistent;
}

void Deps_ScanFileDeps(Deps *this, String base, StringArray arr) {
	for (size_t i = 0; i < arr.len; i++) {
		String tmp;
		if (!String_BeginsWith(&arr.buf[i], tmp = $("#include"))) {
			continue;
		}

		if (arr.buf[i].len < tmp.len + 1) {
			continue;
		}

		String type = String_Slice(&arr.buf[i], tmp.len + 1);
		String_Trim(&type);

		String header;
		bool quotes = false;

		if (type.buf[0] == '<') {
			quotes = false;
			header = String_Between(&arr.buf[i], $("<") , $(">"));
		} else if (type.buf[0] == '"') {
			quotes = true;
			header = String_Between(&arr.buf[i], $("\""), $("\""));
		} else {
			Logger_LogFmt(&logger,
				Logger_Level_Error,
				$("Line '%' not understood."),
				arr.buf[i]);

			continue;
		}

		String_Destroy(&type);

		String_Trim(&header);

		Logger_LogFmt(&logger, Logger_Level_Debug,
			$("Header file '%' found."), header);

		Deps_Type deptype = quotes
			? Deps_Type_Local
			: Deps_Type_System;

		String relPath = Deps_GetFullPath(this, base, header, deptype);

		if (relPath.len > 0) {
			String absPath = Path_Resolve(relPath);

			if (absPath.len > 0) {
				bool scanned = Deps_AddFile(this, absPath);

				if (!scanned) {
					Deps_ScanFile(this, absPath);
				}

				this->node = this->node->parent;
			}

			String_Destroy(&absPath);
		}

		String_Destroy(&relPath);
		String_Destroy(&header);
	}
}

void Deps_ScanFile(Deps *this, String file) {
	Logger_LogFmt(&logger, Logger_Level_Debug, $("Adding '%'..."), file);

	String s = File_GetContents(file);
	StringArray arr = String_Split(&s, '\n');
	String_Destroy(&s);

	String base = Path_GetDirectory(file);

	Deps_ScanFileDeps(this, base, arr);

	String_Destroy(&base);
	StringArray_Destroy(&arr);
}

void Deps_ListSourceFiles(Deps *this) {
	for (size_t i = 0; i < this->deps.len; i++) {
		String path = String_Clone(this->deps.buf[i]->path);

		path.buf[path.len - 1] = 'c';

		if (Path_Exists(path)) {
			Logger_Log(&logger, Logger_Level_Debug, path);
		}

		String_Destroy(&path);
	}
}

void Deps_PrintNode(Deps *this, Deps_Node *node, int indent) {
	if (node == (Deps_Node *) &this->tree.root) {
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
		Deps_PrintNode(this, node->nodes[i], indent + 2);
	}
}

void Deps_PrintTree(Deps *this) {
	Deps_PrintNode(this, this->node, 0);
}

void Deps_Scan(Deps *this) {
	String fullpath = Path_Resolve(this->main);

	if (fullpath.len == 0) {
		Logger_LogFmt(&logger,
			Logger_Level_Error,
			$("Main file '%' not found."),
			this->main);
	} else {
		Deps_AddFile(this, fullpath);
		Deps_ScanFile(this, fullpath);
		this->node = this->node->parent;
	}

	String_Destroy(&fullpath);
}
