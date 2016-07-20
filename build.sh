#!/bin/sh
qmake itemframework.pro -r -spec linux-g++ CONFIG+=debug && make -j 4
exit
