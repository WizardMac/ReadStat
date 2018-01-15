#!/bin/bash

if [[ $CC == 'clang' ]]; then
    ./configure --enable-code-coverage --enable-sanitizers
else
    ./configure
fi
