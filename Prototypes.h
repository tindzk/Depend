#import <String.h>

#import "Utils.h"

#undef self
#define self Prototypes

typedef struct {
	String path;
} Class(self);

def(void, Init);
def(void, Destroy);
def(bool, SetOption, String name, String value);
def(void, Generate);
