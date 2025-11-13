
MAKEFLAGS += "-j $(shell nproc)"

TARGET     := shaq
C_WARNINGS := -Werror -Wall -Wlogical-op -Wextra -Wvla -Wnull-dereference \
              -Wswitch-enum -Wno-deprecated -Wduplicated-cond -Wduplicated-branches \
              -Wshadow -Wpointer-arith -Wcast-qual -Winit-self -Wuninitialized \
              -Wcast-align -Wstrict-aliasing -Wformat=2 -Wmissing-declarations \
              -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings \
              -Wunused-parameter -Wshadow -Wdouble-promotion -Wfloat-equal \
              -Wno-override-init -Wno-error=cpp
C_INCLUDES := -Isrc -Isrc/hgl -Isrc/glad -Isrc/stb -Isrc/imgui -Isrc/ImGuiFileDialog
C_FLAGS    := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -D_DEFAULT_SOURCE -DGLFW_INCLUDE_NONE -fno-strict-aliasing #-fsanitize=address
CPP_FLAGS  := $(C_INCLUDES) --std=c++11
L_FLAGS    := -Llib -lm -lstdc++ -lglfw -ldl -lglfw

ifeq ($(BUILD_TYPE), debug)
	C_FLAGS   += -O0 -g
	CPP_FLAGS += -O0 -g
else ifeq ($(BUILD_TYPE), release)
	C_FLAGS   += -O2 -g -march=native
	CPP_FLAGS += -O2 -g -march=native
endif

ifneq ($(DISABLE_FREETYPE), yes)
C_FLAGS   += -DIMGUI_ENABLE_FREETYPE
CPP_FLAGS += -DIMGUI_ENABLE_FREETYPE $(shell pkg-config --cflags freetype2)
L_FLAGS   += -lfreetype
export DISABLE_FREETYPE
endif

CPP_COMPILE = @parallel -t --tty -j$(shell nproc) g++ -c $(CPP_FLAGS) {1} -o {2}{1/.}.o ::: $(1) ::: $(2)
C_COMPILE = @parallel -t --tty -j$(shell nproc) gcc -c $(C_FLAGS) {1} -o {2}{1/.}.o ::: $(1) ::: $(2)
C_LINK = gcc $(C_FLAGS) $(1) -o $(2) $(L_FLAGS)

.PHONY: shaq imgui font debug release build docs

all: debug

debug: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=debug build

release: prep
	@$(MAKE) --no-print-directory BUILD_TYPE=release build

build: shaq font imgui
	$(call C_LINK, build/*.o build/imgui/*.o, $(TARGET))
	@$(MAKE) docs

shaq:
	$(call CPP_COMPILE, src/*.cpp, build/)
	$(call C_COMPILE, src/*.c src/glad/*.c, build/)

imgui:
ifeq ("$(wildcard build/imgui/*.o)","")
	$(call CPP_COMPILE, src/imgui/*.cpp, build/imgui/)
	$(call CPP_COMPILE, src/ImGuiFileDialog/*.cpp, build/imgui/)
ifneq ($(DISABLE_FREETYPE), yes)
	$(call CPP_COMPILE, src/imgui/misc/freetype/*.cpp, build/imgui/)
endif
endif

font:
	$(LD) -r -z noexecstack -b binary -o build/default_font.o src/fonts/default_font.ttf

docs:
	./$(TARGET) --list-builtins > docs/sel_functions.md

prep:
	@-mkdir -p build 
	@-mkdir -p build/imgui 
	@-mkdir -p docs

clean:
	-@rm build/*.o 2> /dev/null ||:
	-@rm $(TARGET) 2> /dev/null ||:

cleaner:
	-@rm -r build/ 2> /dev/null ||:
	-@rm $(TARGET) 2> /dev/null ||:

