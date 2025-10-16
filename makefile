
MAKEFLAGS += "-j $(shell nproc)"

TARGET     := shaq
C_WARNINGS := -Werror -Wall -Wlogical-op -Wextra -Wvla -Wnull-dereference \
              -Wswitch-enum -Wno-deprecated -Wduplicated-cond -Wduplicated-branches \
              -Wshadow -Wpointer-arith -Wcast-qual -Winit-self -Wuninitialized \
              -Wcast-align -Wstrict-aliasing -Wformat=2 -Wmissing-declarations \
              -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings \
              -Wunused-parameter -Wshadow -Wdouble-promotion -Wfloat-equal \
              -Wno-override-init -Wno-error=cpp
C_INCLUDES := -Isrc -Isrc/hgl -Isrc/glad -Isrc/stb
C_FLAGS    := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -D_DEFAULT_SOURCE -fno-strict-aliasing #-fsanitize=address
CPP_FLAGS  := $(C_INCLUDES) --std=c++11
L_FLAGS    := -Llib -lm -lstdc++ -lglfw -ldl

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
C_LINK = gcc $(C_FLAGS) $(1) -o $(2) $(L_FLAGS)

.PHONY: shaq imgui

all: debug

debug: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=debug build

release: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=release build

profile: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=profile build

build: shaq imgui
	$(call C_LINK, build/*.o build/imgui/*.o, $(TARGET))

shaq:
	$(call CPP_COMPILE, src/*.cpp, build/)
	$(call C_COMPILE, src/*.c src/glad/*.c, build/)

imgui:
ifeq ("$(wildcard build/imgui/*.o)","")
	$(call CPP_COMPILE, src/imgui/*.cpp, build/imgui/)
endif

prep:
	@-mkdir -p build 
	@-mkdir -p build/imgui 

clean:
	-@rm build/*.o 2> /dev/null ||:
	-@rm shaq 2> /dev/null ||:

cleaner:
	-@rm -r build/ 2> /dev/null ||:
	-@rm shaq 2> /dev/null ||:

