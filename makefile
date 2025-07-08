
TARGET := shaq

C_WARNINGS := -Werror -Wall -Wlogical-op -Wextra -Wvla -Wnull-dereference \
			  -Wswitch-enum -Wno-deprecated -Wduplicated-cond -Wduplicated-branches \
			  -Wshadow -Wpointer-arith -Wcast-qual -Winit-self -Wuninitialized \
			  -Wcast-align -Wstrict-aliasing -Wformat=2 -Wmissing-declarations \
			  -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings \
			  -Wunused-parameter -Wshadow -Wdouble-promotion -Wfloat-equal \
			  -Wno-error=cpp 
C_INCLUDES := -Isrc -Iinclude
C_FLAGS    := $(C_WARNINGS) $(C_INCLUDES) --std=c17 -O0 -ggdb3 


SOURCES := src/main.c 	       \
		   src/alloc.c 	       \
		   src/sel/lexer.c     \
		   src/sel/parser.c    \
		   src/sel/typecheck.c \
		   src/sel/builtins.c  \

all:
	$(CC) $(C_FLAGS) $(SOURCES) -o $(TARGET)

clean:
	-rm $(TARGET)

