#import <File.h>
#import <Logger.h>
#import <String.h>

#import "Deps.h"

#define self ManifestWriter

class {
	Deps   *deps;
	Logger *logger;
};

rsdef(self, new, Logger *logger, Deps *deps);
def(void, destroy);
def(void, create);

#undef self
