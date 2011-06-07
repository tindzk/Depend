#import <File.h>
#import <String.h>
#import <Logger.h>
#import <Process.h>
#import <Terminal.h>
#import <Terminal/Prompt.h>

#import "Deps.h"
#import "Utils.h"
#import "Queue.h"

#define self Builder

class {
	Deps *deps;

	Logger *logger;
	Terminal *term;

	bool manifest;
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
};

def(void, Init, Terminal *term, Logger *logger, Deps *deps);
def(void, Destroy);
def(bool, SetOption, RdString name, RdString value);
def(bool, Run);

#undef self
