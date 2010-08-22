#import <Array.h>
#import <String.h>
#import <Process.h>

#import "Deps.h"
#import "Utils.h"

#undef self
#define self Builder

typedef struct {
	String source;
	String output;
} ref(QueueItem);

typedef struct {
	String src;
	String dest;
} ref(DepsMapping);

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

	StringArray *link;
	StringArray *linkpaths;

	Array(ref(DepsMapping), *mappings);
	Array(ref(QueueItem),   *queue);
} self;

def(void, Init, Deps *deps);
def(void, Destroy);
def(bool, SetOption, String name, String value);
def(bool, CreateQueue);
def(void, PrintQueue);
def(bool, Run);
