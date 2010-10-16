#import <String.h>

#import "Deps.h"
#import "Utils.h"
#import "Builder.h"
#import "Prototypes.h"

#undef self
#define self Interface

typedef enum {
	ref(Action_Build),
	ref(Action_ListDeps),
	ref(Action_DepTree),
	ref(Action_PrintQueue),
	ref(Action_Prototypes),
	ref(Action_Help),
	ref(Action_Unsupported)
} ref(Action);

typedef struct {
	ref(Action) action;

	struct {
		Deps deps;
		Builder builder;
		Prototypes proto;
	} private;

	DepsInstance deps;
	BuilderInstance builder;
	PrototypesInstance proto;
} Class(self);

def(void, Init);
def(void, Destroy);
def(void, SetAction, String action);
def(bool, SetOption, String name, String value);
def(bool, Run);
