import ast
import errno
import getopt
import os
import sys

import astor


class RewriteTr(ast.NodeTransformer):
    """Replace tr to QT_TRANSLATE_NOOP
    """
    def visit_Call(self, node):
        self.generic_visit(node)
        # Transform 'tr' into 'QT_TRANSLATE_NOOP' """
        if (isinstance(node.func, ast.Name) and \
           ("tr" == node.func.id) and \
           len(node.args) == 2):
            call = ast.Call(func=ast.Name(id='QT_TRANSLATE_NOOP', ctx=ast.Load()),
                            args=node.args,
                            keywords=[])
            ast.copy_location(call, node)
            # Add lineno & col_offset to the nodes we created
            ast.fix_missing_locations(call)
            return call
         
        # Return the original node if we don't want to change it.
        return node


def mkdir_p(path):
    """Ensure directory ``path`` exists. If needed, parent directories
    are created.
    Adapted from http://stackoverflow.com/a/600612/1539918
    """
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:  # pragma: no cover
            raise


def main(argv):

    input_file = ''
    output_file = ''
    try:
        opts, args = getopt.getopt(argv, "hi:o:", ["ifile=","ofile="])
    except getopt.GetoptError:
        print('RewriteTr.py -i <inputfile> -o <outputfile>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('RewriteTr.py -i <inputfile> -o <outputfile>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            input_file = arg
        elif opt in ("-o", "--ofile"):
            output_file = arg
   
    with open(input_file, "r") as source:
        tree = ast.parse(source.read())

    tree_new = RewriteTr().visit(tree)  
    all_lines = astor.to_source(tree_new)
    
    # if needed, create output directory
    output_dir = os.path.dirname(output_file)
    #print("output_dir [%s]" % output_dir)
    mkdir_p(output_dir)

    # replace the single quotation marks to double quotation marks. It is necessary for lupdate
    with open(output_file, "w") as destination:
        for line in all_lines:
            linenew = line.replace('\'', "\"")
            destination.write(linenew)


if __name__ == "__main__":
    main(sys.argv[1:])
