DIM_IN_BYTES = 44736kb
CC = gcc
EMS = emcc
SRC = src
SRCS = $(wildcard $(SRC)/*.c)
GCC_OBJ = obj
EMS_OBJ = obj
GCC_OBJS = $(patsubst $(SRC)/%.c, $(GCC_OBJ)/%.o, $(SRCS))
EMS_OBJS = $(patsubst $(SRC)/%.c, $(EMS_OBJ)/emscripten_%.o, $(SRCS))
FLAGS =  -Wall -ffast-math -fassociative-math -funsafe-math-optimizations -fno-math-errno -ffinite-math-only -fno-trapping-math
OPEN_MP_FLAGS = -fopenmp
GCC_FLAGS = $(FLAGS) $(OPEN_ACC_MP_FLAGS) -flto -w -Wl,-subsystem,windows -O3 -fbranch-probabilities -fprofile-generate -fprofile-use# -DOPT
GCC_LFLAGS = -static -lmingw32 -lSDL2main -lSDL2 -mwindows -lmingw32 -ldinput8 -lshell32 -lsetupapi -ladvapi32 -luuid -lversion -loleaut32 -lole32 -limm32 -lwinmm -lgdi32 -luser32 -lm -mwindows -Wl,--no-undefined -pipe -Wl,--dynamicbase,--high-entropy-va,--nxcompat,--default-image-base-high -s# -Wl,--subsystem,windows
EMS_FLAGS = $(FLAGS) -Os -s USE_SDL=2 -DNDEBUG
EMS_LFLAGS = --closure 1 -s FILESYSTEM=0 -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=$(DIM_IN_BYTES)
BINDIR = bin
BIN_NAME = ANTS_sdl2
GCC_BIN = $(BINDIR)/$(BIN_NAME)
EMS_BIN = $(BINDIR)/HTML/$(BIN_NAME).js

ifeq ($(OS),Windows_NT)
RM = del /Q /F
CP = copy /Y
ifdef ComSpec
SHELL := $(ComSpec)
endif
ifdef COMSPEC
SHELL := $(COMSPEC)
endif
else
RM = rm -rf
CP = cp -f
endif

all: $(GCC_BIN)

both: $(GCC_BIN) $(EMS_BIN)

gcc: $(GCC_BIN)

$(GCC_BIN): $(GCC_OBJS)
	$(CC) $(GCC_OBJS) -o $@ $(GCC_FLAGS) $(GCC_LFLAGS)

$(GCC_OBJ)/%.o: $(SRC)/%.c
	$(CC) $(GCC_FLAGS) -c $< -o $@

emscripten: $(EMS_BIN)

$(EMS_BIN): $(EMS_OBJS)
	$(EMS) $(EMS_OBJS) -o $@ $(EMS_FLAGS) $(EMS_LFLAGS)

$(EMS_OBJ)/emscripten_%.o: $(SRC)/%.c
	$(EMS) $(EMS_FLAGS) -c $< -o $@

.PHONY : clean

ifeq ($(OS),Windows_NT)
GCC_OBJS := $(subst /,\, $(GCC_OBJS))
EMS_OBJS := $(subst /,\, $(EMS_OBJS))
GCC_BIN := $(subst /,\, $(GCC_BIN))
EMS_BIN := $(subst /,\, $(EMS_BIN))
endif
clean:
	$(RM) $(GCC_BIN).* $(GCC_OBJS) $(EMS_OBJS) $(EMS_BIN)