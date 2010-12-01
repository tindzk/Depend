#import <String.h>

#import "Utils.h"

#define self Prototypes

class {
	String path;
};

def(void, Init);
def(void, Destroy);
def(bool, SetOption, String name, String value);
def(void, Generate);

#undef self
