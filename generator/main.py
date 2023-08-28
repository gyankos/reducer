# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import argparse


# Press the green button in the gutter to run the script.
from GeneratorConfiguration import GeneratorConfiguration

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

    gen = GeneratorConfiguration(args.sigma, args.pattern, args.type, args.n, args.m, args.p, args.l)
    L, N = gen.generate()
    gen.serialize(args.out, L, N)

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
