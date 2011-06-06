#import <Logger.h>
#import <String.h>
#import <Terminal.h>

#import "Deps.h"
#import "Utils.h"
#import "Builder.h"
#import "Prototypes.h"

#define self Interface

set(ref(Action)) {
	ref(Action_Build),
	ref(Action_ListDeps),
	ref(Action_PrintQueue),
	ref(Action_Prototypes),
	ref(Action_Help),
	ref(Action_Unsupported)
};

class {
	ref(Action) action;

	Logger *logger;

	Deps deps;
	Builder builder;
	Prototypes proto;
};

def(void, Init, Terminal *term, Logger *logger);
def(void, Destroy);
def(void, SetAction, RdString action);
def(bool, SetOption, RdString name, RdString value);
def(bool, Run);

#undef self
