#import <File.h>
#import <String.h>
#import <Process.h>

#import "Deps.h"
#import "Utils.h"

#define self Builder

#ifndef Builder_ManifestGapSize
#define Builder_ManifestGapSize 256
#endif

record(ref(QueueItem)) {
	String source;
	String output;
};

record(ref(DepsMapping)) {
	String src;
	String dest;
};

class {
	DepsInstance deps;

	String manifest;
	String output;
	String cc;
	String inclhdr;
	bool dbgsym;
	String std;
	bool blocks;
	int optmlevel;
	bool verbose;

	StringArray *link;
	StringArray *linkpaths;

	Array(ref(DepsMapping), *mappings);
	Array(ref(QueueItem),   *queue);
};

def(void, Init, DepsInstance deps);
def(void, Destroy);
def(bool, SetOption, String name, String value);
def(bool, CreateQueue);
def(void, PrintQueue);
def(bool, Run);

#undef self
