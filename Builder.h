#ifndef BUILDER_H
#define BUILDER_H

#include <Array.h>
#include <String.h>
#include <Process.h>

#include "Deps.h"

#include "Utils.h"

typedef struct {
	String source;
	String output;
} Builder_QueueItem;

typedef struct {
	String src;
	String dest;
} Deps_Mapping;

typedef struct {
	Deps *deps;

	String output;
	String cc;
	String inclhdr;
	bool dbgsym;
	String std;
	bool blocks;
	int optmlevel;
	bool verbose;

	StringArray link;
	StringArray linkpaths;
	Array(Deps_Mapping, mappings);
	Array(Builder_QueueItem, queue);
} Builder;

void Builder_Init(Builder *this, Deps *deps);
void Builder_Destroy(Builder *this);
bool Builder_SetOption(Builder *this, String name, String value);
String Builder_ShrinkPathEx(String shortpath, String path);
String Builder_ShrinkPath(Builder *this, String path);
String Builder_GetOutput(Builder *this, String path);
String Builder_GetSource(String path);
void Builder_AddToQueue(Builder *this, String source, String output);
bool Builder_Compile(Builder *this, String src, String out);
void Builder_Link(Builder *this, StringArray files);
bool Builder_CreateQueue(Builder *this);
void Builder_PrintQueue(Builder *this);
bool Builder_Run(Builder *this);

#endif
