#!/usr/bin/env python3
import os.path
import sys
import time
from pathlib import Path

import subprocess

def run_lydia(arg):
    L = ["/usr/local/bin/lydia", "--file", arg, "-l1", "-s"]
    start = time.perf_counter()
    popen = subprocess.Popen(L, stdout=subprocess.PIPE)
    popen.wait()
    output = str(popen.stdout.read())
    print(output)
    end = time.perf_counter()
    difference = end - start
    difference = difference * 1000
    if popen.returncode == 0:
    		for line in output.split(os.linesep):
    			if "Time elapsed for DFA construction: " in line:
    				t = line.split("Time elapsed for DFA construction: ")[1].split("\\n")[0]
    				difference = float(t[:-2])
    return popen.returncode, difference

def run_aaltaf(arg):
	L = ["./submodules/aaltaf/cmake-build-release/ltlsolver", arg]
	print(" ".join(L))
	start = time.perf_counter()
	popen = subprocess.Popen(L, stdout=subprocess.PIPE)
	popen.wait()
	output = popen.stdout.read()
	print(output)
	end = time.perf_counter()
	difference = end - start
	difference = difference * 1000
	if popen.returncode == 0:
		difference = float(str(output.decode('utf-8')))
	return popen.returncode, difference

if __name__ == "__main__":
	oom_file = open("oom_file.log.txt", "w", buffering=1)
	with open('for_lydia_scan.txt', 'r', buffering=1) as r:
		with open('out.csv', 'a', buffering=1) as the_file:
			for x in r.readlines():
					x = x.strip()
					s = Path(x).stem
					print(Path(x))
					axmodel, t1 = run_lydia(os.path.join(x, "ltlf_ax_model.txt"))
					if axmodel>0:
						print("axmodel: OOM! in "+str(t1)+"ms")
						oom_file.write(os.path.join(x, "ltlf_ax_model.txt")+","+s+",axmodel,"+str(t1))
						oom_file.flush()
					else:
						the_file.write(s+",axmodel,"+str(t1)+os.linesep)
						the_file.flush()
					model,t2 = run_lydia(os.path.join(x, "ltlf_model.txt"))
					if model>0:
						print("model: OOM! in "+str(t2)+"ms")
						oom_file.write(os.path.join(x, "ltlf_model.txt")+","+s+",model,"+str(t2))
						oom_file.flush()
					else:
						the_file.write(s+",model,"+str(t2)+os.linesep)
						the_file.flush()
					rmodel,t3 = run_lydia(os.path.join(x, "ltlf_reduced.txt"))
					if rmodel>0:
						print("rmodel: OOM! in "+str(t3)+"ms")
						oom_file.write(os.path.join(x, "ltlf_reduced.txt")+","+s+",rmodel,"+str(t3))
						oom_file.flush()
					else:
						the_file.write(s+",rmodel,"+str(t3)+os.linesep)
						the_file.flush()
					
					aaxmodel, at1 = run_aaltaf(os.path.join(x, "aaltaf_ax_model.txt"))
					if aaxmodel>0:
						print("axmodel: OOM! in "+str(at1)+"ms")
						oom_file.write(os.path.join(x, "aaltaf_ax_model.txt")+","+s+",aaltaf_axmodel,"+str(at1))
						oom_file.flush()
					else:
						the_file.write(s+",aaltaf_axmodel,"+str(at1)+os.linesep)
						the_file.flush()
					amodel,at2 = run_aaltaf(os.path.join(x, "aaltaf_model.txt"))
					if amodel>0:
						print("model: OOM! in "+str(at2)+"ms")
						oom_file.write(os.path.join(x, "aaltaf_model.txt")+","+s+",aaltaf_model,"+str(at2))
						oom_file.flush()
					else:
						the_file.write(s+",aaltaf_model,"+str(at2)+os.linesep)
						the_file.flush()
					armodel,at3 = run_aaltaf(os.path.join(x, "aaltaf_reduced.txt"))
					if armodel>0:
						print("rmodel: OOM! in "+str(at3)+"ms")
						oom_file.write(os.path.join(x, "aaltaf_reduced.txt")+","+s+",aaltaf_rmodel,"+str(at3))
						oom_file.flush()
					else:
						the_file.write(s+",aaltaf_rmodel,"+str(at3)+os.linesep)
						the_file.flush()
					
					print("")
					print("")
	# lydia_benchmark(sys.argv[1])


