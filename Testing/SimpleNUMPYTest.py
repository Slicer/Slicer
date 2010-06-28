
import os
import sys

# prevents dashboard from truncating output of this test.
print >> sys.stderr, "Enabling CTEST_FULL_OUTPUT\n"
print >> sys.stderr, "\nChecking NUMPY..."
try:
    import numpy
    print >> sys.stderr, numpy.__version__
    print >> sys.stderr, "\t----> import numpy WORKS!"
    
except ImportError:
    print >> sys.stderr, "\t----> import numpy FAILED."
    os._exit(1)

os._exit(0)
