#!/bin/sh
gcc -D CO_DBUG  -fPIC -std=gnu11 -shared -o output/libcask.so *.c -I./ -ldl

