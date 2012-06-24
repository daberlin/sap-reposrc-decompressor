#!/bin/sh -x

g++ -o sap-reposrc-decompressor -O2 -Wall -Wno-maybe-uninitialized sap-*.cpp lib/*.cpp
