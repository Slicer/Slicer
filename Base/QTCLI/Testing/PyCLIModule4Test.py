#!/usr/bin/env python

from __future__ import print_function
import argparse, sys
import numpy as np

# TODO: would be good to set up test driver so `import slicer, vtk` works,
#       to make sure we are actually in Slicer's python.

def main():
    # TODO parse from XML (maybe commontk/ctk-cli)

    parser = argparse.ArgumentParser(description="A test Python CLI")
    parser.add_argument('outputfile', metavar='<outputfile>', help="Output file",
                        nargs='?')
    parser.add_argument('--inputvalue1', metavar='N1', help="Input value 1",
                        required=True, nargs='?', type=int)
    parser.add_argument('--inputvalue2', metavar='N2', help="Input value 2",
                        required=True, nargs='?', type=int)
    parser.add_argument('--operationtype',
                        choices=['Addition', 'Multiplication', 'Fail'],
                        default='Addition')


    args = parser.parse_args()
    operation = args.operationtype

    print(args.outputfile)
    if args.outputfile is None:
        raise Exception("Please specify exactly 1 output file name")

    result = 0
    if operation == 'Addition':
        result = args.inputvalue1 + args.inputvalue2
    elif operation == 'Multiplication':
        result = args.inputvalue1 * args.inputvalue2
    else:
        raise Exception("Unknown OperationType!")

    with open(args.outputfile, "w") as output_f:
        output_f.write(str(result))


if __name__ == '__main__':
    main()
