CC = gcc
CPPFLAGS = -Isrc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g
LDFLAGS =
LDLIBS =

VPATH = src

BIN = minimake
TEST_BIN = unit_test_minimake

SRC = minimake.c
OBJ = ${SRC:.c=.o}

TEST_SRC =
TEST_OBJ = ${TEST_SRC:.c=.o}

.PHONY: all check clean

all: ${BIN}

${BIN}: ${OBJ}

${TEST_BIN}: ${TEST_OBJ}
	${CC} ${LDFLAGS} $^ -lcriterion -o $@

check:
	@echo 'Launch test suite'

clean:
	${RM} ${BIN} ${TEST_BIN} ${OBJ} ${TEST_OBJ}
