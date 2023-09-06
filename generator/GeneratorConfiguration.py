import itertools
# import sys
import os
from string import Formatter
import networkx


def graph_to_patterns(G, sigma, varnames):
    for e in G.edges:
        yield {varnames[0]: sigma[e[0]], varnames[1]: sigma[e[1]]}

def get_variable_names(pattern):
    return [fn for _, fn, _, _ in Formatter().parse(pattern) if fn is not None]

def read_sigma(filename):
    file1 = open(filename, 'r')
    for x in file1.readlines():
        x = x.strip()
        if not (x == "\\n'"):
            yield x.strip()

def read_string(filename):
    with open(filename, 'r') as file:
        return file.read().strip()

def generate_chain(L, varnames, N):
    for chunk in filter(lambda x: len(x) == len(varnames), [L[x:x + len(varnames)] for x in range(0, len(L))]):
        if N <= 0:
            break
        yield dict(zip(varnames, chunk))
        N = N-1

def loop_chain(iter, varnames):
    L = list(iter)
    d = dict()
    d[varnames[0]] = L[-1][varnames[1]]
    d[varnames[1]] = L[0][varnames[0]]
    L.append(d)
    return L

def instantiate_patterns(pattern, iter):
    for x in iter:
        yield pattern.format_map(x)


def write_sigma(L, filename):
    with open(filename, "w") as f:
        for x in L:
            f.write(x+os.linesep)

class GeneratorConfiguration:
    def __init__(self, sigma, pattern, type, n, m, p, l):
        self.l = l
        self.p = p
        self.m = m
        self.n = n
        self.type = type
        self.pattern = pattern
        self.sigma = list(read_sigma(sigma))
        write_sigma(self.sigma, sigma)
        if isinstance(self.l, str):
            self.l = eval(self.l)

    def getSigma(self):
        return self.sigma

    def generate(args, pattern=None):
        if pattern is None:
            pattern = read_string(args.pattern)
        variables = get_variable_names(pattern)
        L = []
        N = 0
        ls = []
        if args.type == "path":
            N = max(min(int(args.n), len(args.sigma)), 2)
            instantiate = generate_chain(args.sigma, variables, N)
            ls = list(instantiate)
        if args.type == "chain":
            N = max(min(int(args.n), len(args.sigma)), 2)
            instantiate = generate_chain(args.sigma, variables, N - 1)
            ls = loop_chain(instantiate, variables)
        if "graph" in args.type:
            G = networkx.Graph()
            if args.type == "dense_gnm_random_graph":
                N = int(args.n)
                m = int(args.m)
                G = networkx.dense_gnm_random_graph(N, m, create_using=networkx.DiGraph)
            elif args.type == "complete_graph":
                N = int(args.n)
                G = networkx.complete_graph(N, create_using=networkx.DiGraph)
            elif args.type == "circulant_graph":
                N = int(args.n)
                l = list(int(x) for x in args.l)
                G = networkx.circulant_graph(N, l, create_using=networkx.DiGraph)
            elif args.type == "gnr_graph":
                N = int(args.n)
                p = float(args.p)
                G = networkx.gnr_graph(N, p, create_using=networkx.DiGraph)
            ls = list(graph_to_patterns(G, args.sigma, variables))
        return instantiate_patterns(pattern, ls), N

    def serialize(args, out, L, N):
        dir = out + "_" + args.type + "_" + str(N)
        file = "model.txt"
        if not os.path.exists(dir):
            os.makedirs(dir)
        filename = os.path.join(dir, file)
        # with open(filename, 'w') as f:
        #     for line in L:
        #         f.write(f"{line}\n")
        return dir, file