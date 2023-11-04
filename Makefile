# Compiler setup
CC := gcc
LD := gcc
CFLAGS := -Wall -O2
LDFLAGS :=
LIBS := -lm  # Math library included by default

# Cross-compiler setup
CROSS_COMPILE ?= 0
ifeq ($(CROSS_COMPILE), 1)
  CC := x86_64-w64-mingw32-gcc
  LD := x86_64-w64-mingw32-gcc
endif

# Source files and objects
SOURCES := rftg.c init.c engine.c ai.c net.c loadsave.c tui.c 
OBJECTS := $(SOURCES:.c=.o)
DEPS := $(OBJECTS:.o=.d)

# Phony targets
.PHONY: all clean debug windows

# Default build target
all: release

# Release build
release: rftg

# Debug build
debug: CFLAGS += -g
debug: rftg

# Linking the executable
rftg: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

# Compiling source files
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# Clean up
clean:
	rm -f $(OBJECTS) rftg $(DEPS)

# Cross-compile for Windows
windows:
	$(MAKE) CROSS_COMPILE=1
