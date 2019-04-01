from __future__ import print_function

import os
import sys

# prevents dashboard from truncating output of this test.
print("Enabling CTEST_FULL_OUTPUT\n", file=sys.stderr)
print("\nChecking NUMPY...", file=sys.stderr)
try:
    import numpy
    print(numpy.__version__, file=sys.stderr)
    print("\t----> import numpy WORKS!", file=sys.stderr)

except ImportError:
    print("\t----> import numpy FAILED.", file=sys.stderr)
    os._exit(1)

os._exit(0)
