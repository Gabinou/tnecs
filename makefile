

CC := tcc 
DIR_INCLUDE = include


INCLUDE_ALL = -I. -I${DIR_INCLUDE}

# FLAGS_ERROR := -Wall -pedantic-errors
FLAGS_ERROR :=
ifeq ($(OS), Windows_NT)
	EXEC := test.exe
else
	EXEC := CodenameFireSaga_make.bin 
endif 

# FLAGS_BUILD_TYPE = -O3 -DNDEBUG #Release
FLAGS_BUILD_TYPE = -O0 -g  #Debug

CFLAGS := ${INCLUDE_ALL} ${FLAGS_BUILD_TYPE} ${LIBS_THIRD} ${FLAGS_ERROR}

.PHONY: all $(EXEC)
all: $(EXEC)
SOURCES_SIMPLECS := simplecs.c
SOURCES_TEST := test.c
TARGETS_SIMPLECS := $(SOURCES_SIMPLECS:.c=.o)

# $(TARGETS_SIMPLECS) : $(SOURCES_SIMPLECS) ; $(CC) $< -c -o $@
$(EXEC): $(SOURCES_TEST); $(CC) $< -o $@  $(CFLAGS)  $(INCLUDE_ALL)  ${FLAGS_BUILD_TYPE} ${FLAGS_ERROR}

.PHONY: wclean
wclean: ; del /q /s *.o *.a *.exe build\\*.txt
.PHONY: clean
clean: ; @echo "Cleaning Simplecs"; rm -frv $(TARGETS_SIMPLECS) $(EXEC)
