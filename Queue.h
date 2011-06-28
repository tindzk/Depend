#import <String.h>
#import <Logger.h>

#import "Deps.h"
#import "Utils.h"

#define self Queue

exc(RuntimeError)

record(ref(Item)) {
	RdString source;
	String   output;
	bool     built;
	pid_t    pid;
};

Array(ref(Item), ref(Items));

class {
	Logger       *logger;
	Deps         *deps;
	ref(Items)   *queue;
	MappingArray *mappings;
	size_t       ofs;
	size_t       running;
};

static alwaysInline def(size_t, getRunning) {
	return this->running;
}

static alwaysInline def(size_t, getOffset) {
	return this->ofs;
}

static alwaysInline def(size_t, getTotal) {
	return this->queue->len;
}

rsdef(self, new, Logger *logger, Deps *deps, MappingArray *mappings);
def(void, destroy);
def(void, create);
def(bool, hasNext);
def(ref(Item) *, getNext);
def(void, setBuilding, ref(Item) *item, pid_t pid);
def(void, setBuilt, pid_t pid);
def(StringArray *, getLinkingFiles);

#undef self
