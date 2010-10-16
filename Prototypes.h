#import <String.h>

#import "Utils.h"

#undef self
#define self Prototypes

class(self) {
	String path;
};

def(void, Init);
def(void, Destroy);
def(bool, SetOption, String name, String value);
def(void, Generate);
