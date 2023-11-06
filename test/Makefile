EXE := test

DIRS := . .. ../../SVG

DEPS := \
	$(wildcard $(addsuffix /*.h,${DIRS}) $(addsuffix /*.cpp,${DIRS})) \
	Makefile

CPPS := $(filter %.cpp,${DEPS})

.PHONY: all
all: ${EXE}

${EXE}: ${DEPS}
	@g++ -std=c++11 -Wall -O3 -Wfatal-errors -Werror \
	${CPPS} -o ${EXE} $(addprefix -I ,${DIRS})

.PHONY: run
run: ${EXE}
	@./${EXE}

.PHONY: files
files:
	@echo ${DEPS}

clean:
	rm -f ${EXE}
	rm -f *.svg
