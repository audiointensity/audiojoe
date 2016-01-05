import sys
from antlr4 import *
from joeLexer import joeLexer
from joeParser import joeParser

def main(argv):
    input = FileStream(argv[1])
    lexer = joeLexer(input)
    stream = CommonTokenStream(lexer)
    parser = joeParser(stream)
    tree = parser.joefile()

if __name__ == '__main__':
    main(sys.argv)
