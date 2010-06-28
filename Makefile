# Compile with Clang using the `config.h' configuration file and
# enable blocks.
CC = clang -include config.h

# Enable warnings.
CFLAGS = -W -Wall -pipe

# Include debugging symbols.
# This will add file names and line numbers to your backtraces.
CFLAGS += -g

# Disable optimizations.
CFLAGS += -O0

# Jivai uses a special C variant. Therefore these parameters
# are necessary.
CFLAGS += -std=gnu99 -fblocks -lBlocksRuntime

# Define the library path.
LIBPATH = $(JIVAI_PATH)

# Only build these modules.
LIBFILES += $(LIBPATH)/{Process,Path,Array,Char,String,Backtrace,Integer,Memory,ExceptionManager,Tree,File,BufferedStream,FileStream,Hex,Logger}.c

# Enable human-readable backtraces (requires BFD).
LIBFILES += $(LIBPATH)/BFD.c -lbfd

# Set the library path.
LIBFILES += -I$(LIBPATH)

all:
	$(CC) -o jutils.bin $(CFLAGS) *.c $(LIBFILES)