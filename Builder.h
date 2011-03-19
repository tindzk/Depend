#import <File.h>
#import <String.h>
#import <Logger.h>
#import <Process.h>

#import "Deps.h"
#import "Utils.h"

#define self Builder

// @exc RuntimeError

record(ref(QueueItem)) {
	String source;
	String output;
};

record(ref(DepsMapping)) {
	String src;
	String dest;
};

Array(ref(DepsMapping), MappingArray);
Array(ref(QueueItem),   QueueArray);

class {
	DepsInstance deps;

	Logger *logger;

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
	MappingArray *mappings;
	QueueArray *queue;
};

def(void, Init, Logger *logger, DepsInstance deps);
def(void, Destroy);
def(bool, SetOption, RdString name, RdString value);
def(bool, CreateQueue);
def(void, PrintQueue);
def(bool, Run);

#undef self
