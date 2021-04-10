

CC := tcc # tcc, gcc 

DIR_INCLUDE = include
INCLUDE_ALL = -I. -I${DIR_INCLUDE}

# FLAGS_ERROR := -Wall -pedantic-errors
FLAGS_ERROR :=

EXEC := test.exe

# FLAGS_BUILD_TYPE = -O3 -DNDEBUG #Release
# FLAGS_BUILD_TYPE = -O0 -g  #Debug
FLAGS_BUILD_TYPE =  #Debug

CFLAGS := ${INCLUDE_ALL} ${FLAGS_BUILD_TYPE} ${LIBS_THIRD} ${FLAGS_ERROR}

.PHONY: all $(EXEC)  
all: astyle $(EXEC) run 
SOURCES_SIMPLECS := simplecs.c
SOURCES_TEST := test.c
HEADERS := $(wildcard *.h)
SOURCES_ALL := $(SOURCES_SC_TIMER) $(SOURCES_TEST) $(SOURCES_SIMPLECS) 
TARGETS_SIMPLECS := $(SOURCES_SIMPLECS:.c=.o)
# SOURCES_SC_TIMER := sc_time.c
# TARGETS_SC_TIMER := $(SOURCES_SC_TIMER:.c=.o)

.PHONY : run
run: $(EXEC); $(EXEC)

.PHONY : astyle
astyle: $(HEADERS) $(SOURCES_ALL); astyle --style=java --indent=spaces=4 --indent-switches --pad-oper --pad-comma --pad-header --unpad-paren  --align-pointer=middle --align-reference=middle --add-braces --add-one-line-braces --attach-return-type --convert-tabs --suffix=none *.h *.c

$(TARGETS_SIMPLECS) : $(SOURCES_SIMPLECS) ; $(CC) $< -c -o $@ 
# $(TARGETS_SC_TIMER) : $(SOURCES_SC_TIMER) ; $(CC) $< -c -o $@ 
$(EXEC): $(SOURCES_TEST) $(TARGETS_SIMPLECS); $(CC) $< $(TARGETS_SIMPLECS) -o $@ $(CFLAGS)

.PHONY: wclean
wclean: ; del /q /s *.o *.a *.exe build\\*.txt
.PHONY: clean
clean: ; @echo "Cleaning Simplecs" & rm -frv $(TARGETS_SIMPLECS) $(TARGETS_SC_TIMER) $(EXEC)
