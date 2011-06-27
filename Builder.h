#import <String.h>
#import <Logger.h>
#import <Process.h>
#import <Terminal.h>
#import <Terminal/Prompt.h>

#import <Signal.h>
#import <EventLoop.h>

#import "Deps.h"
#import "Utils.h"
#import "Queue.h"
#import "ManifestWriter.h"

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
	size_t workers;

	StringArray *link;
	StringArray *linkpaths;
	MappingArray *mappings;

	Queue queue;
};

def(void, Init, Terminal *term, Logger *logger, Deps *deps);
def(void, Destroy);
def(bool, map, RdString value);
def(void, setOutput, RdString value);
def(void, setCompiler, RdString value);
def(void, setInclHeader, RdString value);
def(void, setManifest, bool value);
def(void, setDebuggingSymbols, bool value);
def(void, setStandard, RdString value);
def(void, setBlocks, bool value);
def(void, setOptimLevel, u16 value);
def(void, setWorkers, u16 value);
def(void, addLink, RdString value);
def(void, addLinkPath, RdString value);
def(void, setVerbose, bool value);
def(bool, Run);

#undef self
