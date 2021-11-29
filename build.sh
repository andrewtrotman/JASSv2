#!/bin/bash
cd build
cmake ..
make
cd anytime
cp _pyjass.so ../output
cp pyjass.py ../output
