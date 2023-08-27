#!/usr/bin/env python3
import sys
import time

if __name__ == "__main__":
	from logaut import ltl2dfa
	from pylogics.parsers import parse_ltl
	data = ""
	with open(sys.argv[1], 'r') as file:
    		data = file.read().replace('\n', '')
	formula = parse_ltl(data)
	start = time.perf_counter()
	dfa = ltl2dfa(formula, backend="lydia")
	end = time.perf_counter()
	difference = end - start
	difference = difference * 1000
	print(dfa.to_graphviz())
	print("Time: {}ms".format(difference))
