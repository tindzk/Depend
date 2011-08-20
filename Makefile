JIVAI_PATH ?= ../Jivai

# Define the library path.
LIBPATH = $(JIVAI_PATH)/src

# Compile with Clang using the default configuration.
CC = clang -include $(JIVAI_PATH)/config.h

# Enable warnings.
CFLAGS = -W -Wall -pipe

# As some SSE2 instructions are generated that are not 16-byte
# aligned, the program will crash when SSE2 is enabled. A quick
# solution may be to replace all MOVAPS- with MOVUPS instructions.
CFLAGS += -march=generic

# Include debugging symbols.
# This will add file names and line numbers to your backtraces.
CFLAGS += -g

# Disable optimizations.
CFLAGS += -O0

# Jivai uses a special C variant. Therefore these parameters
# are necessary.
CFLAGS += -std=gnu99 -fblocks

CFLAGS += -L/usr/lib/gcc/i686-pc-linux-gnu/4.6.1
CFLAGS += -B/usr/lib/gcc/i686-pc-linux-gnu/4.6.1

# Needed so that Jivai libraries know where to find the manifest file.
CFLAGS += -I.

# Set the library path.
CFLAGS += -I$(LIBPATH)

# Only build these modules.
LIBFILES += $(LIBPATH)/{Main,Application,Channel,Signal,Memory,Folder{,/Expander},System,Kernel,Block,Process,Path,Char,String{,Stream,Reader},Backtrace,Integer,Exception,Tree,Terminal{,/Controller,/Prompt,/Buffer},Unicode,Ecriture{,/Parser},File,BufferedStream,Hex,Logger,ELF,DWARF,Buffer,LEB128,Memory/{Map,Libc,Logger},Task,EventLoop,ChannelWatcher,EventQueue,FPU,CPU,MemoryMappedFile,Locale,HashTable,MurmurHash3}.c

all:
	if test -f Depend.exe; then ./update-manifest.sh; fi

	bash -c '$(CC) -o Depend.tmp $(CFLAGS) \
		-DNamespace=\"Jivai\" $(LIBFILES) -UNamespace \
		-DNamespace=\"main\" *.c'

	mv Depend.tmp Depend.exe
