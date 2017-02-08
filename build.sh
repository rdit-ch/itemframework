#!/bin/sh
qmake itemframework.pro -r -spec linux-clang CONFIG+=debug && make -j 4
exit
