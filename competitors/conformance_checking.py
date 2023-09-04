#!/usr/bin/env python3
import os.path
import sys
import time
from pathlib import Path

import subprocess

def stream_process(process):
    go = process.poll() is None
    for line in process.stdout:
        print(line)
    return go


def run(dataset, models):
	zL = ["/home/giacomo/projects/knobab2_loggen/cmake-build-release/knobab2-benchmarking", dataset, "knobab.csv", "10", "-d"]
	for x in models:
		L.append(x)
	return " ".join(L)

datasets = ["/home/giacomo/Scaricati/data_preparer/mal-api-2019(2)/all.tab_9.tab",
"/home/giacomo/Scaricati/data_preparer/mal-api-2019(2)/all.tab_81.tab",
"/home/giacomo/Scaricati/data_preparer/mal-api-2019(2)/all.tab_729.tab"]

if __name__ == "__main__":
	oom_file = open("oom_file.log.txt", "a", buffering=1)
	L = []
	with open('for_lydia_scan.txt', 'r', buffering=1) as r:
		for x in r.readlines():
			x = x.strip()
			L.append(os.path.join(x, "powerdecl_reduced.txt"))
			L.append(os.path.join(x, "powerdecl.txt"))
	print("Obtained declarative models:")
	print(L)
	with open("knobab_sh.sh", "w", buffering=1) as f:
		f.write("#!/bin/bash"+os.linesep)
		for dataset in datasets:
			f.write(run(dataset, L)+os.linesep)
			f.flush()
