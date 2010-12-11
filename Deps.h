#import <Path.h>
#import <Tree.h>
#import <Array.h>
#import <String.h>
#import <Directory.h>

#import "Utils.h"

#define self Deps

set(ref(Type)) {
	ref(Type_System),
	ref(Type_Local)
};

record(ref(Module)) {
	String name;
	StringArray *exc;
};

Array(ref(Module), ref(Modules));
Array(size_t,      ref(ModuleOffsets));

record(ref(Component)) {
	Tree_Define(ref(Component));

	String path;
	ref(ModuleOffsets) *modules;
};

class {
	String main;

	StringArray *paths;
	StringArray *include;

	/* Dependency tree. */
	Tree tree;
	ref(Component) *component;

	/* All modules flattened. */
	ref(Modules) *modules;
};

def(void, Init);
def(void, Destroy);
sdef(void, DestroyNode, ref(Component) *node);
def(bool, SetOption, String name, String value);
def(StringArray *, GetIncludes);
def(ref(Modules) *, GetModules);
def(ref(Component) *, GetComponent);
def(StringArray *, GetPaths);
def(void, ListSourceFiles);
def(void, PrintTree);
def(void, Scan);

#undef self
