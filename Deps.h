#import <Path.h>
#import <Tree.h>
#import <Array.h>
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

typedef struct {
	String main;

	StringArray *include;

	/* Dependency tree. */
	Tree tree;
	ref(Node) *node;

	/* All top-level deps flattened. */
	Array(ref(Node) *, *deps);
} self;

def(void, Init);
def(void, Destroy);
void ref(DestroyNode)(ref(Node) *node);
def(bool, SetOption, String name, String value);
static def(void, ScanFile, String file);
def(void, ListSourceFiles);
def(void, PrintTree);
def(void, Scan);
