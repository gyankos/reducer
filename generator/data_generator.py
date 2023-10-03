import os.path
import math
from urllib.request import urlopen
import subprocess
from pathlib import Path

from GeneratorConfiguration import GeneratorConfiguration
bin = os.path.join("cmake-build-release","reducer")

import subprocess

def run(args):
    L = [bin]
    for x in args:
        L.append(x)
    print(" ".join(L))
    popen = subprocess.Popen(L, stdout=subprocess.PIPE)
    popen.wait()
    output = popen.stdout.read()
    print(str(output))

def generate_method_and_run(min, count, sigma_file,case,l, pattern,patterns, exponent, expand=None):
    g = GeneratorConfiguration(sigma_file, None, case, min, min * min / 2, 0.5, l)
    L, N = g.generate(patterns)
    if expand is not None:
        L = expand(L, g)
    dir, file = g.serialize(os.path.join("dataset", "malware_" + pattern), L, N)
    run([os.path.join(dir, file),
         "--name", str(Path(dir).stem),
         "--ltlf", os.path.join(dir, "ltlf_model.txt"),
         "--ltlfax", os.path.join(dir, "ltlf_ax_model.txt"),
         "--ltlfreduced", os.path.join(dir, "ltlf_reduced.txt"),
         "--aaltaf", os.path.join(dir, "aaltaf_model.txt"),
         "--aaltafax", os.path.join(dir, "aaltaf_ax_model.txt"),
         "--aaltafreduced", os.path.join(dir, "aaltaf_reduced.txt"),
         "-p", os.path.join(dir, "powerdecl.txt"),
         "-e", os.path.join(dir, "powerdecl_reduced.txt")
         ])
    return dir


def expand_with_first(L, g):
    L = list(L)
    L.append('Existence("'+g.getSigma()[0]+'")')
    return L

if __name__ == "__main__":
    print("Downloading sigma file")
    data = urlopen("https://raw.githubusercontent.com/ocatak/malware_api_class/master/ApiIndex.txt")
    count = 0
    sigma_file = str(os.path.join("dataset", "malware_sigma.txt"))
    with open(sigma_file, "w") as f:
        for line in data: # files are iterable
            f.write(str(line).split("=")[0][2:]+os.linesep)
            count = count + 1
    maxexp = math.floor(math.log2(count))
    exponent = 2
    min = exponent
    cases = ["chain", "complete_graph", "circulant_graph"]
    l = [0,1,2,3]

    # patterns = {"r": 'Response("{a}" "{b}")',
    #             }
    patterns = {"cr": 'ChainResponse("{a}" "{b}")',
                "p": 'Precedence("{a}" "{b}")',
                "r": 'Response("{a}" "{b}")',
                "re": 'RespExistence("{a}" "{b}")',
                "rexc": 'RespExistence("{a}" "{b}")'+os.linesep+'ExclChoice("{a}" "{b}")'
                }
    L = []
    while min < count:
        for pattern in patterns:
            for case in cases:
                if (pattern == "re") or (pattern == "rexc"):
                    L.append(generate_method_and_run(min, count, sigma_file, case, l, pattern, patterns[pattern], exponent,
                                                  expand=expand_with_first))
                else:
                    L.append(generate_method_and_run(min, count, sigma_file, case, l, pattern, patterns[pattern], exponent))
        L.append(generate_method_and_run(min, count, sigma_file, "path", l, "crar",
                                      'ChainResponse("{a}" "{a}")' + os.linesep + 'AltResponse("{a}" "{a}")', exponent))
        L.append(generate_method_and_run(min, count, sigma_file, "path", l, "cr2",
                                         'ChainResponse("a" "{a}")',
                                         exponent))
        min = min * exponent
    min = exponent
    # with open("for_lydia_scan.txt", "w") as f:
    #     for x in L:
    #         f.write(x+os.linesep)


