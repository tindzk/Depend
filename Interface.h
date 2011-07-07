#import <Logger.h>
#import <String.h>
#import <Terminal.h>

#import "Deps.h"
#import "Builder.h"

#define self Interface

set(ref(Action)) {
	ref(Action_Unknown),
	ref(Action_Build),
	ref(Action_ListDeps),
	ref(Action_Help)
};

class {
	Logger *logger;

	Deps deps;
	Builder builder;
};

def(void, init, Terminal *term, Logger *logger);
def(void, destroy);
def(bool, run, RdStringArray *args, RdString base);

#undef self
