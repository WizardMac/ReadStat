set PATH=C:\msys64\usr\bin;%PATH%
set PATH=C:\msys64\mingw64\bin;%PATH%

set CC=x86_64-w64-mingw32-gcc

REM echo useful info
bash --login -c "$CC -v"

REM appveyor msys configure workaround "exec 0</dev/null"
bash --login -c "cd `cygpath '%CD%'`; exec 0</dev/null; ./autogen.sh"
bash --login -c "cd `cygpath '%CD%'`; exec 0</dev/null; ./configure"
bash --login -c "cd `cygpath '%CD%'`; make"
