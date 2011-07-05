#import <Path.h>
#import <File.h>
#import <Tree.h>
#import <Array.h>
#import <String.h>
#import <Logger.h>
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
Array(size_t,      ref(ComponentOffsets));

record(ref(Component)) {
	String source;
	String header;
	ref(ModuleOffsets)    *modules;
	ref(ComponentOffsets) *deps;
	bool build;
};

Array(ref(Component), ref(Components));

class {
	String          main;
	Logger          *logger;
	StringArray     *include;
	ref(Modules)    *modules;
	ref(Components) *components;
};

rsdef(self, new, Logger *logger);
def(void, destroy);
def(bool, setOption, RdString name, RdString value);
def(void, add, RdString value);
def(void, setMain, RdString s);
def(RdString, getMain);
def(void, addInclude, RdString s);
def(void, scan);

static alwaysInline def(StringArray *, getIncludes) {
	return this->include;
}

static alwaysInline def(ref(Modules) *, getModules) {
	return this->modules;
}

static alwaysInline def(ref(Components) *, getComponents) {
	return this->components;
}

#undef self
