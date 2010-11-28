#import <Path.h>
#import <Tree.h>
#import <Array.h>
#import <String.h>
#import <Directory.h>

#import "Utils.h"

#undef self
#define self Deps

set(ref(Type)) {
	ref(Type_System),
	ref(Type_Local)
};

record(ref(Node)) {
	Tree_Define(ref(Node));

	String path;
	String module;
};

typedef Array(ref(Node) *, DepsArray);

class {
	String main;

	StringArray *include;

	/* Dependency tree. */
	Tree tree;
	ref(Node) *node;

	/* All top-level deps flattened. */
	DepsArray *deps;
};

def(void, Init);
def(void, Destroy);
void ref(DestroyNode)(ref(Node) *node);
def(bool, SetOption, String name, String value);
def(StringArray *, GetIncludes);
def(DepsArray *, GetDeps);
def(void, ListSourceFiles);
def(void, PrintTree);
def(void, Scan);
