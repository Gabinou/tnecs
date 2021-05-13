

COMPILER := gcc # tcc, gcc, clang

# OS AND Processor detection 
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
LINUX_PRE := ./
WIN_PRE := 

# FLAGS_BUILD_TYPE = -O3 -DNDEBUG #Release
FLAGS_BUILD_TYPE = -O0 -g -fprofile-arcs -ftest-coverage #Debug

# FLAGS_ERROR := -Wall -pedantic-errors
FLAGS_ERROR := -w
INCLUDE_ALL := -I. 

# astyle detection: isASTYLE is empty unless astyle exists
ifeq ($(OS_FLAG),WIN32)
	EXTENSION := $(WIN_EXT)
    PREFIX := $(WIN_PRE)
	isASTYLE := $(shell where astyle)
    CFLAGS := ${INCLUDE_ALL} ${FLAGS_BUILD_TYPE} ${FLAGS_ERROR}
else
	EXTENSION := $(LINUX_EXT)
    PREFIX := $(LINUX_PRE)
	isASTYLE := $(shell type astyle)
    CFLAGS := ${INCLUDE_ALL} ${FLAGS_BUILD_TYPE} ${FLAGS_ERROR} -lm
endif

# $(info $$isASTYLE is [$(isASTYLE)])
$(info $$EXTENSION is [$(EXTENSION)])

ifeq ($(isASTYLE),)
	ASTYLE :=
else
	ASTYLE := astyle 
endif

EXEC := $(PREFIX)test$(EXTENSION)
EXEC_TCC := $(PREFIX)test_tcc$(EXTENSION)
EXEC_GCC := $(PREFIX)test_gcc$(EXTENSION)
EXEC_CLANG := $(PREFIX)test_clang$(EXTENSION)
EXEC_ALL := ${EXEC} ${EXEC_TCC} ${EXEC_GCC} ${EXEC_CLANG}

.PHONY: all 
all: ${ASTYLE} $(TARGETS_FLECS) flecs $(EXEC) run 
SOURCES_TNECS := tnecs.c
SOURCES_TEST := test.c
SOURCES_FLECS := flecs.c
HEADERS := $(wildcard *.h)
SOURCES_ALL := $(SOURCES_TEST) $(SOURCES_TNECS) 
TARGETS_TNECS := $(SOURCES_TNECS:.c=.o)
TARGETS_FLECS := $(SOURCES_FLECS:.c=.o)
TARGETS_TNECS_GCC := $(SOURCES_TNECS:.c=_gcc.o)
TARGETS_TNECS_TCC := $(SOURCES_TNECS:.c=_tcc.o)
TARGETS_TNECS_CLANG := $(SOURCES_TNECS:.c=_clang.o)
TARGETS_ALL := ${TARGETS_TNECS} ${TARGETS_FLECS} ${TARGETS_TNECS_GCC} ${TARGETS_TNECS_TCC} ${TARGETS_TNECS_CLANG}
.PHONY: compile_test
compile_test: ${ASTYLE} ${EXEC_TCC}  ${EXEC_GCC} ${EXEC_CLANG} run_tcc run_gcc run_clang

.PHONY : cov
cov: ; lcov -c --directory . --output-file main_coverage.info ; genhtml main_coverage.info --output-directory out

.PHONY : run
run: $(EXEC); $(EXEC)
.PHONY : flecs # Only compiles for gcc or clang
flecs: $(TARGETS_FLECS)
.PHONY : run_tcc
run_tcc: $(EXEC_TCC) ; $(EXEC_TCC)
.PHONY : run_gcc
run_gcc: $(EXEC_GCC) ; $(EXEC_GCC)
.PHONY : run_clang
run_clang: $(EXEC_CLANG) ; $(EXEC_CLANG)
.PHONY : astyle
astyle: $(HEADERS) $(SOURCES_ALL); astyle --style=java --indent=spaces=4 --indent-switches --pad-oper --pad-comma --pad-header --unpad-paren  --align-pointer=middle --align-reference=middle --add-braces --add-one-line-braces --attach-return-type --convert-tabs --suffix=none *.h *.c

$(TARGETS_FLECS) : $(SOURCES_FLECS) ; $(COMPILER) $< -c -o $@

$(TARGETS_TNECS) : $(SOURCES_TNECS) ; $(COMPILER) $< -c -o $@ $(CFLAGS)
$(TARGETS_TNECS_CLANG) : $(SOURCES_TNECS) ; clang $< -c -o $@ 
$(TARGETS_TNECS_GCC) : $(SOURCES_TNECS) ; gcc $< -c -o $@
$(TARGETS_TNECS_TCC) : $(SOURCES_TNECS) ; tcc $< -c -o $@ 

$(EXEC): $(SOURCES_TEST) $(TARGETS_TNECS) $(TARGETS_FLECS) ; $(COMPILER) $< $(TARGETS_TNECS) $(TARGETS_FLECS) -o $@ $(CFLAGS)
$(EXEC_TCC): $(SOURCES_TEST) $(TARGETS_TNECS_TCC); tcc $< $(TARGETS_TNECS_TCC) -o $@ $(CFLAGS)
$(EXEC_GCC): $(SOURCES_TEST) $(TARGETS_TNECS_GCC); gcc $< $(TARGETS_TNECS_GCC) -o $@ $(CFLAGS)
$(EXEC_CLANG): $(SOURCES_TEST) $(TARGETS_TNECS_CLANG); clang $< $(TARGETS_TNECS_CLANG) -o $@ $(CFLAGS)



.PHONY: clean
clean: ; @echo "Cleaning tnecs" & rm -frv $(TARGETS_ALL) $(EXEC_ALL) 
.PHONY: cleancov
cleancov: ; @echo "Cleaning tnecs coverage tests" & rm -frv out *.gcda *.gcno *.gcov *.info
.PHONY: cleanall
cleanall: clean cleancov
