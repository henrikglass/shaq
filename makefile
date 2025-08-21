
MAKEFLAGS += "-j $(shell nproc)"

TARGET     := shaq
C_WARNINGS := -Werror -Wall -Wlogical-op -Wextra -Wvla -Wnull-dereference \
              -Wswitch-enum -Wno-deprecated -Wduplicated-cond -Wduplicated-branches \
              -Wshadow -Wpointer-arith -Wcast-qual -Winit-self -Wuninitialized \
              -Wcast-align -Wstrict-aliasing -Wformat=2 -Wmissing-declarations \
              -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings \
              -Wunused-parameter -Wshadow -Wdouble-promotion -Wfloat-equal \
              -Wno-override-init -Wno-error=cpp
C_INCLUDES := -Isrc -Isrc/hgl -Isrc/glad -Isrc/stb -Isrc/tracy
C_FLAGS    := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -D_DEFAULT_SOURCE -fno-strict-aliasing #-fsanitize=address
CPP_FLAGS  := $(C_INCLUDES) --std=c++11
L_FLAGS    := -Llib -lm -lglfw -lstdc++

ifeq ($(BUILD_TYPE), debug)
	C_FLAGS   += -O0 -g
	CPP_FLAGS += -O0 -g
else ifeq ($(BUILD_TYPE), release)
	C_FLAGS   += -O2 -g -march=native
	CPP_FLAGS += -O2 -g -march=native
else ifeq ($(BUILD_TYPE), profile)
	C_FLAGS   += -O2 -g -march=native -DTRACY_ENABLE 
	CPP_FLAGS += -O2 -g -march=native -DTRACY_ENABLE 
endif

CPP_COMPILE = @parallel -t --tty -j$(shell nproc) g++ -c $(CPP_FLAGS) {1} -o {2}{1/.}.o ::: $(1) ::: $(2)
C_COMPILE = @parallel -t --tty -j$(shell nproc) gcc -c $(C_FLAGS) {1} -o {2}{1/.}.o ::: $(1) ::: $(2)
C_LINK = $(CC) $(C_FLAGS) $(1) -o $(2) $(L_FLAGS)

.PHONY: shaq tracy imgui

all: debug

debug: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=debug build

release: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=release build

profile: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=profile build

prep:
	@-mkdir build 
	@-mkdir build/tracy 
	@-mkdir build/imgui 

build: shaq tracy imgui
	$(call C_LINK, build/*.o build/tracy/*.o build/imgui/*.o, $(TARGET))

shaq:
	$(call CPP_COMPILE, src/*.cpp, build/)
	$(call C_COMPILE, src/*.c src/glad/*.c, build/)

tracy:
ifeq ("$(wildcard build/tracy/*.o)","")
	$(call CPP_COMPILE, src/tracy/*.cpp, build/tracy/)
endif

imgui:
ifeq ("$(wildcard build/imgui/*.o)","")
	$(call CPP_COMPILE, src/imgui/*.cpp, build/imgui/)
endif

clean:
	-rm build/*.o
	-rm shaq

cleaner: clean
	-rm build/tracy/*
	-rm build/imgui/*
	-rm build/*
	-rm -r build/

