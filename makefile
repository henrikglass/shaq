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
C_FLAGS    := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -O0 -ggdb3 -fno-strict-aliasing
L_FLAGS    := -lm


SOURCES := src/main.c 	       \
		   src/alloc.c 	       \
		   src/sel.c           \

all: shaq sel

shaq:
	gcc $(C_FLAGS) $(SOURCES) -o shaq $(L_FLAGS)

sel:
	gcc $(C_FLAGS) -Isrc test/test_sel.c src/sel.c src/alloc.c -o sel $(L_FLAGS)

clean:
	-rm shaq
	-rm sel

