CC = gcc
CPPFLAGS = -MMD -Isrc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g -fsanitize=address
LDFLAGS = -fsanitize=address
LDLIBS =

VPATH = src

BIN = minimake
TEST_BIN = unit_test_minimake

SRC = minimake.c vector.c parser.c vars_rules.c var_sub.c build_targets.c exec_command.c log_command.c stack.c
OBJ = ${SRC:.c=.o}
DEP = ${SRC:.c=.d}

TEST_SRC =
TEST_OBJ = ${TEST_SRC:.c=.o}

.PHONY: all check clean

all: ${BIN}

${BIN}: ${OBJ}

${TEST_BIN}: ${TEST_OBJ}
	${CC} ${LDFLAGS} $^ -lcriterion -o $@

check: ${BIN}
	./tests/check.sh

clean:
	${RM} ${BIN} ${TEST_BIN} ${OBJ} ${TEST_OBJ} ${DEP}

-include ${DEP}

#END
