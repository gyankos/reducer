#!/usr/bin/bash
# Script required for specifiying the correct location of the libraries at the OS level
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
python3 competitors/all_benchmark.py
