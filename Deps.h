#import <Path.h>
#import <Tree.h>
#import <Array.h>
#import <Directory.h>
#import <StringArray.h>

#import "Utils.h"

#undef self
#define self Deps

typedef enum {
	ref(Type_System),
	ref(Type_Local)
} ref(Type);

typedef struct ref(Node) {
	Tree_Define(ref(Node));
	String path;
} ref(Node);

typedef Array(ref(Node) *, DepsArray);

typedef struct {
	String main;

	StringArray *include;

	/* Dependency tree. */
	Tree tree;
	ref(Node) *node;

	/* All top-level deps flattened. */
	DepsArray *deps;
} Class(self);

def(void, Init);
def(void, Destroy);
void ref(DestroyNode)(ref(Node) *node);
def(bool, SetOption, String name, String value);
def(StringArray *, GetIncludes);
def(DepsArray *, GetDeps);
def(void, ListSourceFiles);
def(void, PrintTree);
def(void, Scan);
