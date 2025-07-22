MAKEFLAGS += "-j $(shell nproc)"

.PHONY: shaq sel clean

C_WARNINGS := -Werror -Wall -Wlogical-op -Wextra -Wvla -Wnull-dereference \
			  -Wswitch-enum -Wno-deprecated -Wduplicated-cond -Wduplicated-branches \
			  -Wshadow -Wpointer-arith -Wcast-qual -Winit-self -Wuninitialized \
			  -Wcast-align -Wstrict-aliasing -Wformat=2 -Wmissing-declarations \
			  -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings \
			  -Wunused-parameter -Wshadow -Wdouble-promotion -Wfloat-equal \
			  -Wno-error=cpp 
C_INCLUDES := -Iinclude
C_FLAGS    := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -O0 -ggdb3 -D_DEFAULT_SOURCE -fno-strict-aliasing
L_FLAGS    := -lm -lglfw


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


all: shaq sel

shaq:
	gcc $(C_FLAGS) src/main.c $(SOURCES) -o shaq $(L_FLAGS)

sel:
	gcc $(C_FLAGS) -Isrc test/test_sel.c $(SOURCES) -o sel $(L_FLAGS)

clean:
	-rm shaq
	-rm sel

