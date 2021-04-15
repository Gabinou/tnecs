
# NEED OS DETECTOR. 

CC := tcc # tcc, gcc 


LINUX_EXT := .bin
WIN_EXT := .exe
EXEC := test.exe
EXEC_TCC := test_tcc.exe
EXEC_GCC := test_gcc.exe
EXEC_CLANG := test_clang.exe

# FLAGS_BUILD_TYPE = -O3 -DNDEBUG #Release
# FLAGS_BUILD_TYPE = -O0 -g  #Debug
FLAGS_BUILD_TYPE =  #Debug

# FLAGS_ERROR := -Wall -pedantic-errors
FLAGS_ERROR :=
INCLUDE_ALL := -I. 

CFLAGS := ${INCLUDE_ALL} ${FLAGS_BUILD_TYPE} ${FLAGS_ERROR}

.PHONY: all 
all: astyle $(EXEC) run 
SOURCES_SIMPLECS := simplecs.c
SOURCES_TEST := test.c
HEADERS := $(wildcard *.h)
SOURCES_ALL := $(SOURCES_TEST) $(SOURCES_SIMPLECS) 
TARGETS_SIMPLECS := $(SOURCES_SIMPLECS:.c=.o)
TARGETS_SIMPLECS_GCC := $(SOURCES_SIMPLECS:.c=_gcc.o)
TARGETS_SIMPLECS_TCC := $(SOURCES_SIMPLECS:.c=_tcc.o)
TARGETS_SIMPLECS_CLANG := $(SOURCES_SIMPLECS:.c=_clang.o)

.PHONY: compile  
compile: astyle ${EXEC_TCC} ${EXEC_GCC} ${EXEC_CLANG} run_tcc run_gcc run_clang

.PHONY : run
run: $(EXEC); $(EXEC)

.PHONY : run2
run: $(EXEC_TCC) $(EXEC_GCC) $(EXEC_CLANG) ; $(EXEC_TCC) && $(EXEC_GCC) && $(EXEC_CLANG)
.PHONY : astyle
astyle: $(HEADERS) $(SOURCES_ALL); astyle --style=java --indent=spaces=4 --indent-switches --pad-oper --pad-comma --pad-header --unpad-paren  --align-pointer=middle --align-reference=middle --add-braces --add-one-line-braces --attach-return-type --convert-tabs --suffix=none *.h *.c

$(TARGETS_SIMPLECS) : $(SOURCES_SIMPLECS) ; $(CC) $< -c -o $@

$(TARGETS_SIMPLECS_CLANG) : $(SOURCES_SIMPLECS) ; clang $< -c -o $@ 
$(TARGETS_SIMPLECS_GCC) : $(SOURCES_SIMPLECS) ; gcc $< -c -o $@
$(TARGETS_SIMPLECS_TCC) : $(SOURCES_SIMPLECS) ; tcc $< -c -o $@ 

$(EXEC): $(SOURCES_TEST) $(TARGETS_SIMPLECS); $(CC) $< $(TARGETS_SIMPLECS) -o $@ $(CFLAGS)
$(EXEC_TCC): $(SOURCES_TEST) $(TARGETS_SIMPLECS_TCC); tcc $< $(TARGETS_SIMPLECS_TCC) -o $@ $(CFLAGS)
$(EXEC_GCC): $(SOURCES_TEST) $(TARGETS_SIMPLECS_GCC); gcc $< $(TARGETS_SIMPLECS_GCC) -o $@ $(CFLAGS)
$(EXEC_CLANG): $(SOURCES_TEST) $(TARGETS_SIMPLECS_CLANG); clang $< $(TARGETS_SIMPLECS_CLANG) -o $@ $(CFLAGS)

.PHONY: wclean
wclean: ; del /q /s *.o *.a *.exe build\\*.txt
.PHONY: clean
clean: ; @echo "Cleaning Simplecs" & rm -frv $(TARGETS_SIMPLECS) $(TARGETS_SC_TIMER) $(EXEC)
