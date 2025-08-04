MAKEFLAGS += "-j $(shell nproc)"

.PHONY: shaq sel clean cleaner

C_WARNINGS     := -Werror -Wall -Wlogical-op -Wextra -Wvla -Wnull-dereference \
			      -Wswitch-enum -Wno-deprecated -Wduplicated-cond -Wduplicated-branches \
			      -Wshadow -Wpointer-arith -Wcast-qual -Winit-self -Wuninitialized \
			      -Wcast-align -Wstrict-aliasing -Wformat=2 -Wmissing-declarations \
			      -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings \
			      -Wunused-parameter -Wshadow -Wdouble-promotion -Wfloat-equal \
				  -Wno-override-init \
			      -Wno-error=cpp 
C_INCLUDES     := -Iinclude
C_FLAGS        := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -D_DEFAULT_SOURCE -fno-strict-aliasing
DEBUG_FLAGS    := -O0 -ggdb3
RELEASE_FLAGS  := -O2 -march=native
L_FLAGS        := -Llib -lm -lglfw -limgui -lstdc++

SOURCES := src/alloc.c 	       \
		   src/str.c           \
		   src/selc.c          \
		   src/selvm.c         \
		   src/shaq_core.c     \
		   src/shader.c        \
		   src/uniform.c       \
		   src/texture.c       \
		   src/util.c          \
		   src/io.c            \
		   src/glad/glad.c     \
		   src/gl_util.c       \
		   src/renderer.c      \
		   src/gui.c   		   \
		   src/log.c      	   \


all: debug sel

debug: lib/libimgui.a
	g++ -Wall -Wextra -Iinclude -Iinclude/imgui -O2 -c src/imguic.cpp -o imguic.o
	gcc $(C_FLAGS) $(DEBUG_FLAGS) src/main.c $(SOURCES) -o shaq imguic.o $(L_FLAGS)
	-rm imguic.o

release: lib/libimgui.a
	g++ -Wall -Wextra -Iinclude -Iinclude/imgui -O2 -c src/imguic.cpp -o imguic.o
	gcc $(C_FLAGS) $(RELEASE_FLAGS) src/main.c $(SOURCES) -o shaq imguic.o $(L_FLAGS)
	-rm imguic.o

#sel: lib/libimgui.a
#	g++ -Wall -Wextra -Iinclude -Iinclude/imgui -c src/imguic.cpp -o imguic.o
#	gcc $(C_FLAGS) -Isrc test/test_sel.c $(SOURCES) -o sel imguic.o $(L_FLAGS)
#	rm imguic.o

lib/libimgui.a:
	-mkdir lib
	g++ -Wall -Wextra -Iinclude -Iinclude/imgui -Isrc/imgui -O2 -c src/imgui/*.cpp
	ar cr libimgui.a *.o
	-rm *.o
	mv libimgui.a lib

clean:
	-rm shaq
	-rm sel

cleaner:
	-rm lib/*
	-rmdir lib
	-rm shaq
	-rm sel

