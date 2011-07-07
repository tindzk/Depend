#import <Array.h>
#import <String.h>

record(DepsMapping) {
	String src;
	String dest;
};

Array(DepsMapping, MappingArray);
