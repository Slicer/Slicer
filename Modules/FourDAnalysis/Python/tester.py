#! /Users/junichi/igtdev/slicer/Slicer3-lib/python-build/bin/python2.5

import sys
import scipy, scipy.optimize, scipy.io, numpy
import re
import FourDAnalysis as fda

if len(sys.argv) < 4:
    sys.exit()

Path          = sys.argv[1]
InputCsvFile  = sys.argv[2]
OutputCsvFile = sys.argv[3]


inputCurve    = scipy.io.read_array(InputCsvFile, separator=',')
outputCurve   = inputCurve

caexec = fda.CurveAnalysisExecuter(Path)
result = caexec.Execute(inputCurve, outputCurve)

for i, v, in result.iteritems():
    sys.stderr.write(' %5s     : %f\n'  % (i, v) )



