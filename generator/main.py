# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import argparse
import itertools
# import sys
from string import Formatter
import networkx


def graph_to_patterns(G, sigma, varnames):
    for e in G.edges:
        yield {varnames[0]: sigma[e[0]], varnames[1]: sigma[e[1]]}

def get_variable_names(pattern):
    return [fn for _, fn, _, _ in Formatter().parse(pattern) if fn is not None]

def read_sigma(filename):
    file1 = open(filename, 'r')
    return [x.strip() for x in file1.readlines()]

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



# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='Model Generator',
        description='This generates a model for the specific alphabet and argument',
        epilog='(c) 2022-2023')
    parser.add_argument('sigma', metavar='sigma', type=str, help='Alphabet file, where each element is line separated')
    parser.add_argument('pattern', metavar='pattern', type=str, help='Pattern to be instantiated to this specific file')
    parser.add_argument('type', metavar='type', type=str, help='Type of model to generate from the previous configuration')
    parser.add_argument('out', metavar='out', type=str, help='Name for the generated model')
    parser.add_argument('-n', metavar='n', type=int, help='number of nodes', required=False)
    parser.add_argument('-m', metavar='m', type=int, help='number of edges', required=False)
    parser.add_argument('-p', metavar='p', type=float, help='probability', required=False)
    parser.add_argument('-l', metavar='l', type=str, help='args', required=False)
    args = parser.parse_args()

    sigma = read_sigma(args.sigma)
    pattern = read_string(args.pattern)
    variables = get_variable_names(pattern)
    L = []
    N = 0
    ls = []
    if args.type == "chain":
        N = max(min(int(args.n), len(sigma)), 2)
        instantiate = generate_chain(sigma, variables, N-1)
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
            l = list(int(x) for x in eval(args.l))
            G = networkx.circulant_graph(N, l, create_using=networkx.DiGraph)
        elif args.type == "gnr_graph":
            N = int(args.n)
            p = float(args.p)
            G = networkx.gnr_graph(N, p, create_using=networkx.DiGraph)
        ls = list(graph_to_patterns(G, sigma, variables))
    L = instantiate_patterns(pattern, ls)
    with open(args.out+"_"+args.type+"_"+str(N)+".txt", 'w') as f:
        for line in L:
            f.write(f"{line}\n")

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
