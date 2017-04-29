#!/bin/bash

~/src/afl-2.41b/afl-g++ -std=c++11 -I../include -O2 -g -o sajson-fuzz ../example/main.cpp
