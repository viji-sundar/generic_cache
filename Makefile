CC = gcc
#OPT = -O3 --std=c99
OPT = -g
WARN = -Werror
CFLAGS = $(OPT) $(WARN)
BUILD = build
MKH   = python scripts/mkh.py
EXE_NAME = exec

# List all your .c files here
C_SRC = $(wildcard *.c)
LINK_C_SRC  = $(addprefix $(BUILD)/, $(C_SRC))

# List corresponding compiled object files here (.o files)
C_OBJ = $(LINK_C_SRC:.c=.o)

# List corresponding proto files here (*_proto.h files)
C_PROTO = $(LINK_C_SRC:.c=_proto.h)

# default rule
all: clean build_dir mkh build link soft_link
	@echo "Build complete!"

clean:
	rm -rf $(BUILD)
	rm -rf $(EXE_NAME)

build_dir:
	mkdir -p $(BUILD)

mkh: $(C_PROTO)

build: $(C_OBJ)

link: $(C_OBJ)
	$(CC) $(CFLAGS) $(C_OBJ) -lm -o $(BUILD)/$(EXE_NAME)

soft_link: $(EXE_NAME)

#### Patterns BEGIN ###########

$(BUILD)/%.o:
	$(CC) $(CFLAGS) -c $*.c -lm -o $(BUILD)/$*.o

$(BUILD)/%_proto.h:
	$(MKH) $*.c > $(BUILD)/$*_proto.h

$(EXE_NAME):
	ln -s $(BUILD)/$(EXE_NAME) $(EXE_NAME)

#### Patterns END #############
