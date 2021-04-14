

COMPILER := tcc # tcc, gcc 

# OS AND Process detection 
ifeq ($(OS),Windows_NT)
    OS_FLAG := WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        PROCESSOR_FLAG = AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            PROCESSOR_FLAG := AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            PROCESSOR_FLAG := IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OS_FLAG := LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        PROCESSOR_FLAG := OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        PROCESSOR_FLAG := AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        PROCESSOR_FLAG := IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        PROCESSOR_FLAG := ARM
    endif
endif

$(info $$OS_FLAG is [${OS_FLAG}])
$(info $$PROCESSOR_FLAG is [${PROCESSOR_FLAG}])



LINUX_EXT := .bin
WIN_EXT := .exe
# astyle detection: isASTYLE is empty unless astyle exists
ifeq ($(OS_FLAG),WIN32)
	EXTENSION := $(WIN_EXT)
	isASTYLE := $(shell where astyle)
else
	EXTENSION := $(LINUX_EXT)
	isASTYLE := $(shell type astyle)
endif
ifeq ($(isASTYLE),)
	ASTYLE :=
else
	ASTYLE := astyle 
endif

$(info $$isASTYLE is [$(isASTYLE)])
$(info $$EXTENSION is [$(EXTENSION)])

EXEC := test$(EXTENSION)
EXEC_TCC := test_tcc$(EXTENSION)
EXEC_GCC := test_gcc$(EXTENSION)
EXEC_CLANG := test_clang$(EXTENSION)

# FLAGS_BUILD_TYPE = -O3 -DNDEBUG #Release
# FLAGS_BUILD_TYPE = -O0 -g  #Debug
FLAGS_BUILD_TYPE =  #Debug

# FLAGS_ERROR := -Wall -pedantic-errors
FLAGS_ERROR :=
INCLUDE_ALL := -I. 

CFLAGS := ${INCLUDE_ALL} ${FLAGS_BUILD_TYPE} ${FLAGS_ERROR}

.PHONY: all 
all: ${ASTYLE} $(EXEC) run 
SOURCES_TNECS := tnecs.c
SOURCES_TEST := test.c
HEADERS := $(wildcard *.h)
SOURCES_ALL := $(SOURCES_TEST) $(SOURCES_TNECS) 
TARGETS_TNECS := $(SOURCES_TNECS:.c=.o)
TARGETS_TNECS_GCC := $(SOURCES_TNECS:.c=_gcc.o)
TARGETS_TNECS_TCC := $(SOURCES_TNECS:.c=_tcc.o)
TARGETS_TNECS_CLANG := $(SOURCES_TNECS:.c=_clang.o)

.PHONY: compile_test
compile_test: ${ASTYLE} ${EXEC_TCC} run_tcc ${EXEC_GCC} run_gcc  ${EXEC_CLANG} run_clang

.PHONY : run
run: $(EXEC); $(EXEC)
.PHONY : run_tcc
run_tcc: $(EXEC_TCC) ; $(EXEC_TCC)
.PHONY : run_gcc
run_gcc: $(EXEC_GCC) ; $(EXEC_GCC)
.PHONY : run_clang
run_clang: $(EXEC_CLANG) ; $(EXEC_CLANG)

.PHONY : astyle
astyle: $(HEADERS) $(SOURCES_ALL); astyle --style=java --indent=spaces=4 --indent-switches --pad-oper --pad-comma --pad-header --unpad-paren  --align-pointer=middle --align-reference=middle --add-braces --add-one-line-braces --attach-return-type --convert-tabs --suffix=none *.h *.c

$(TARGETS_TNECS) : $(SOURCES_TNECS) ; $(COMPILER) $< -c -o $@
$(TARGETS_TNECS_CLANG) : $(SOURCES_TNECS) ; clang $< -c -o $@ 
$(TARGETS_TNECS_GCC) : $(SOURCES_TNECS) ; gcc $< -c -o $@
$(TARGETS_TNECS_TCC) : $(SOURCES_TNECS) ; tcc $< -c -o $@ 

$(EXEC): $(SOURCES_TEST) $(TARGETS_TNECS); $(COMPILER) $< $(TARGETS_TNECS) -o $@ $(CFLAGS)
$(EXEC_TCC): $(SOURCES_TEST) $(TARGETS_TNECS_TCC); tcc $< $(TARGETS_TNECS_TCC) -o $@ $(CFLAGS)
$(EXEC_GCC): $(SOURCES_TEST) $(TARGETS_TNECS_GCC); gcc $< $(TARGETS_TNECS_GCC) -o $@ $(CFLAGS)
$(EXEC_CLANG): $(SOURCES_TEST) $(TARGETS_TNECS_CLANG); clang $< $(TARGETS_TNECS_CLANG) -o $@ $(CFLAGS)

.PHONY: wclean
wclean: ; del /q /s *.o *.a *.exe build\\*.txt
.PHONY: clean
clean: ; @echo "Cleaning Simplecs" & rm -frv $(TARGETS_TNECS) $(TARGETS_SC_TIMER) $(EXEC)
