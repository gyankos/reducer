#!/usr/bin/env python3
import os.path
import sys
import time
from pathlib import Path

def lydia_benchmark(filename, name1, name2, out):
	with open('out.csv', 'a') as the_file:
		from logaut import ltl2dfa
		from pylogics.parsers import parse_ltl
		data = ""
		print(filename)
		with open(filename, 'r') as file:
			data = (file.read().replace('\n', ' ').replace('_',''))
		print("parsing..")
		formula = parse_ltl(data)
		print("DFA generator..")
		start = time.perf_counter()
		dfa = ltl2dfa(formula, backend="lydia")
		end = time.perf_counter()
		difference = end - start
		difference = difference * 1000
		# print(dfa.to_graphviz())
		print("\tTime: {}ms".format(difference))
		the_file.write(name1+","+name2+","+str(difference)+os.linesep)



if __name__ == "__main__":
	lydia_benchmark(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
	# lydia_benchmark(sys.argv[1])


