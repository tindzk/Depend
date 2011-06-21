#import <Logger.h>
#import <String.h>
#import <Terminal.h>

#import "Deps.h"
#import "Utils.h"
#import "Builder.h"

#define self Interface

set(ref(Action)) {
	ref(Action_Build),
	ref(Action_ListDeps),
	ref(Action_Help),
	ref(Action_Unsupported)
};

class {
	ref(Action) action;

	Logger *logger;

	Deps deps;
	Builder builder;
};

def(void, Init, Terminal *term, Logger *logger);
def(void, Destroy);
def(void, SetAction, RdString action);
def(bool, SetOption, RdString name, RdString value);
def(bool, Run);

#undef self
