#!/bin/bash

if [[ $CC == 'clang' ]]; then
    ./configure --enable-code-coverage
else
    ./configure
fi
