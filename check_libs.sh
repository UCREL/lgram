#!/bin/bash

CHK_SQL=`gcc -lsqlite3 2>&1 | grep "cannot find" | wc -l`
CHK_PCRE=`gcc -lpcre 2>&1 | grep "cannot find" | wc -l`

if [ $CHK_SQL -ge 1 ]; then
        echo "ERROR: cannot find libsqlite3"
        exit 1
fi

if [ $CHK_PCRE -ge 1 ]; then
        echo "ERROR: cannot find libpcre"
        exit 1
fi

exit 0
