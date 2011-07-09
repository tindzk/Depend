#import <Array.h>
#import <String.h>

record(DepsMapping) {
	String src;
	String dest;
	String namespace;
};

Array(DepsMapping, MappingArray);
