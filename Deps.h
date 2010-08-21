#ifndef DEPS_H
#define DEPS_H

#include <Path.h>
#include <Tree.h>
#include <Array.h>
#include <StringArray.h>

#include "Utils.h"

typedef enum {
	Deps_Type_System,
	Deps_Type_Local
} Deps_Type;

typedef struct _Deps_Node {
	Tree_Define(_Deps_Node);
	String path;
} Deps_Node;

typedef struct {
	String main;

	StringArray *include;

	/* Dependency tree. */
	Tree tree;
	Deps_Node *node;

	/* All top-level deps flattened. */
	Array(Deps_Node *, *deps);
} Deps;

void Deps_Init(Deps *this);
void Deps_Destroy(Deps *this);
void Deps_DestroyNode(Deps_Node *node);
bool Deps_SetOption(Deps *this, String name, String value);
String Deps_GetLocalPath(__unused Deps *this, String base, String file);
String Deps_GetSystemPath(Deps *this, String file);
String Deps_GetFullPath(Deps *this, String base, String file, Deps_Type type);
bool Deps_AddFile(Deps *this, String absPath);
void Deps_ScanFile(Deps *this, String file);
void Deps_ListSourceFiles(Deps *this);
void Deps_PrintNode(Deps *this, Deps_Node *node, int indent);
void Deps_PrintTree(Deps *this);
void Deps_Scan(Deps *this);

#endif
