
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
SOURCES_TNECS := simplecs.c
SOURCES_TEST := test.c
HEADERS := $(wildcard *.h)
SOURCES_ALL := $(SOURCES_TEST) $(SOURCES_TNECS) 
TARGETS_TNECS := $(SOURCES_TNECS:.c=.o)
TARGETS_TNECS_GCC := $(SOURCES_TNECS:.c=_gcc.o)
TARGETS_TNECS_TCC := $(SOURCES_TNECS:.c=_tcc.o)
TARGETS_TNECS_CLANG := $(SOURCES_TNECS:.c=_clang.o)

.PHONY: compile_test
compile_test: astyle ${EXEC_TCC} ${EXEC_GCC} ${EXEC_CLANG} ; run_tcc run_gcc run_clang

.PHONY : run
run: $(EXEC); $(EXEC)
.PHONY : run_tcc
run_tcc: $(EXEC_TCC)  ; $(EXEC_TCC)
.PHONY : run_gcc
run_gcc: $(EXEC_GCC) ; $(EXEC_GCC)
.PHONY : run_clang
run_clang: $(EXEC_CLANG) ; $(EXEC_CLANG)

.PHONY : astyle
astyle: $(HEADERS) $(SOURCES_ALL); astyle --style=java --indent=spaces=4 --indent-switches --pad-oper --pad-comma --pad-header --unpad-paren  --align-pointer=middle --align-reference=middle --add-braces --add-one-line-braces --attach-return-type --convert-tabs --suffix=none *.h *.c

$(TARGETS_TNECS) : $(SOURCES_TNECS) ; $(CC) $< -c -o $@
$(TARGETS_TNECS_CLANG) : $(SOURCES_TNECS) ; clang $< -c -o $@ 
$(TARGETS_TNECS_GCC) : $(SOURCES_TNECS) ; gcc $< -c -o $@
$(TARGETS_TNECS_TCC) : $(SOURCES_TNECS) ; tcc $< -c -o $@ 

$(EXEC): $(SOURCES_TEST) $(TARGETS_TNECS); $(CC) $< $(TARGETS_TNECS) -o $@ $(CFLAGS)
$(EXEC_TCC): $(SOURCES_TEST) $(TARGETS_TNECS_TCC); tcc $< $(TARGETS_TNECS_TCC) -o $@ $(CFLAGS)
$(EXEC_GCC): $(SOURCES_TEST) $(TARGETS_TNECS_GCC); gcc $< $(TARGETS_TNECS_GCC) -o $@ $(CFLAGS)
$(EXEC_CLANG): $(SOURCES_TEST) $(TARGETS_TNECS_CLANG); clang $< $(TARGETS_TNECS_CLANG) -o $@ $(CFLAGS)

.PHONY: wclean
wclean: ; del /q /s *.o *.a *.exe build\\*.txt
.PHONY: clean
clean: ; @echo "Cleaning Simplecs" & rm -frv $(TARGETS_TNECS) $(TARGETS_SC_TIMER) $(EXEC)
