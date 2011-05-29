JIVAI_PATH ?= ../Jivai

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
LIBFILES += $(LIBPATH)/{Main,Application,Channel,Signal,Memory,Directory,System,Kernel,Block,Process,Path,Char,String{,Stream,Reader},Backtrace,Integer,Exception,Tree,Terminal{,/Controller},Ecriture{,/Parser},File,BufferedStream,FileStream,Hex,Logger,ELF,DWARF,Buffer,LEB128,Memory/{Map,Libc,Logger}}.c

# Set the library path.
LIBFILES += -I$(LIBPATH)

all:
	if test -f Depend.exe; then ./update-manifest.sh; fi
	bash -c "$(CC) -o Depend.tmp $(CFLAGS) *.c $(LIBFILES)"
	mv Depend.tmp Depend.exe
