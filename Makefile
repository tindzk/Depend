JIVAI_PATH ?= ../Jivai

LIBFILES += -L/usr/lib/gcc/i686-pc-linux-gnu/4.6.1
LIBFILES += -B/usr/lib/gcc/i686-pc-linux-gnu/4.6.1

# Compile with Clang using the default configuration.
CC = clang -include $(JIVAI_PATH)/config.h

# Enable warnings.
CFLAGS = -W -Wall -pipe

# Include debugging symbols.
# This will add file names and line numbers to your backtraces.
CFLAGS += -g

# Disable optimizations.
CFLAGS += -O0

# Jivai uses a special C variant. Therefore these parameters
# are necessary.
CFLAGS += -std=gnu99 -fblocks

# Define the library path.
LIBPATH = $(JIVAI_PATH)/src

# Needed so that Jivai libraries know where to find the manifest file.
LIBFILES += -I.

# Only build these modules.
LIBFILES += $(LIBPATH)/{Main,Application,Channel,Signal,Memory,Directory,System,Kernel,Block,Process,Path,Char,String{,Stream,Reader},Backtrace,Integer,Exception,Tree,Terminal{,/Controller,/Prompt,/Buffer},Unicode,Ecriture{,/Parser},File,BufferedStream,FileStream,Hex,Logger,ELF,DWARF,Buffer,LEB128,Memory/{Map,Libc,Logger},Task,EventLoop,ChannelWatcher,EventQueue,SocketServer,SocketConnection,Socket,FPU,CPU,MemoryMappedFile,Locale,HashTable,MurmurHash3}.c

# Set the library path.
LIBFILES += -I$(LIBPATH)

all:
	if test -f Depend.exe; then ./update-manifest.sh; fi

	bash -c '$(CC) -o Depend.tmp $(CFLAGS)   \
		-DNamespace=\"main\" *.c -UNamespace \
		-DNamespace=\"Jivai\"                \
		$(LIBFILES)'

	mv Depend.tmp Depend.exe
