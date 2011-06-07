#import <String.h>
#import <Logger.h>

#import "Deps.h"
#import "Utils.h"

#define self Queue

// @exc RuntimeError

record(ref(Item)) {
	RdString source;
	String   output;
};

Array(ref(Item), ref(Items));

class {
	Logger       *logger;
	Deps         *deps;
	ref(Items)   *queue;
	MappingArray *mappings;
	size_t       ofs;
};

rsdef(self, new, Logger *logger, Deps *deps, MappingArray *mappings);
def(void, destroy);
def(void, create);
def(bool, hasNext);
def(ref(Item), getNext);
def(size_t, getTotal);
def(StringArray *, getLinkingFiles);

#undef self
