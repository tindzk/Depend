JIVAI_PATH ?= ../Jivai

# Compile with Clang using the default configuration.
CC = clang -include Manifest.h -include $(JIVAI_PATH)/config.h

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

# Only build these modules.
LIBFILES += $(LIBPATH)/{Pool,Directory,Runtime,Kernel,Block,Process,Path,Char,String{,Stream},Backtrace,Integer,Memory,Exception,Tree,Terminal{,/Controller},Typography,File,BufferedStream,FileStream,Hex,Logger}.c

# Enable human-readable backtraces (requires BFD).
LIBFILES += $(LIBPATH)/BFD.c -lbfd

# Set the library path.
LIBFILES += -I$(LIBPATH)

all:
	if test -f jutils.bin; then ./update-manifest.sh; fi
	bash -c "$(CC) -o jutils.tmp $(CFLAGS) *.c $(LIBFILES)"
	mv jutils.tmp jutils.bin
