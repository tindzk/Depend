#import <File.h>
#import <String.h>

#import "Utils.h"

#define self Prototypes

class {
	String path;
};

def(void, Init);
def(void, Destroy);
def(bool, SetOption, RdString name, RdString value);
def(void, Generate);

#undef self
