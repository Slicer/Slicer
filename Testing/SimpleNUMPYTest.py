
import os

# prevents dashboard from truncating output of this test.
print "Enabling CTEST_FULL_OUTPUT\n"
print "\nChecking NUMPY..."
try:
    import numpy
    print numpy.__version__
    print "\t----> import numpy WORKS!"
    
except ImportError:
    print "\t----> import numpy FAILED."
    os._exit(1)

os._exit(0)
