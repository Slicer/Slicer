
# prevents dashboard from truncating output of this test.
print "Enabling CTEST_FULL_OUTPUT\n"
print "\nChecking NUMPY..."
try:
    import numpy
    print "\t----> import numpy WORKS!"
    
except ImportError:
    print "\t----> import numpy FAILED."
    #os._exit(-1)
    Slicer.tk.eval('exit 1')

Slicer.tk.eval('exit 0')
