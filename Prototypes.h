#import <String.h>

#import "Utils.h"

#define self Prototypes

class {
	String path;
};

def(void, Init);
def(void, Destroy);
def(bool, SetOption, ProtString name, ProtString value);
def(void, Generate);

#undef self
