#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'

FAILURES=0
TOTAL=0

function check()
{
    for f in tests/files_return_$1/* ; do
        TOTAL=$((TOTAL+1))
        OPTS=$(head -n 1 $f)
        OUTPUT=$(tail -n +2 $f)
        MY_OUTPUT=$(./minimake $OPTS 2> /dev/null)
        MY_RETURN_CODE=$?
        if [[ $MY_RETURN_CODE -eq $1 ]]; then
            if [[ -z $OUTPUT ]]; then
                echo $f: PASS
            elif [[ $OUTPUT = $MY_OUTPUT ]]; then
                echo $f: PASS
            else
                printf "$f: ${RED}FAIL${NC}: differents outputs\n"
                FAILURES=$((FAILURES+1))
            fi
        fi

        if [[ $MY_RETURN_CODE -ne $1 ]] ; then
            FAILURES=$((FAILURES+1))
            printf "$f: ${RED}FAIL${NC}"
            printf ": returned $MY_RETURN_CODE, expected $1\n"
            if [[ -n $OUTPUT ]] &&  [[ $OUTPUT != $MY_OUTPUT ]]; then
                printf "$f: ${RED}FAIL${NC}: differents outputs\n"
            fi
        fi
    done
}

check 0
check 2

echo $FAILURES fails / $TOTAL tests
echo Success rate: $((100 * (TOTAL - FAILURES) / TOTAL)) %
