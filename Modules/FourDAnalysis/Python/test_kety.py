#! /Users/junichi/igtdev/slicer/Slicer3-lib/python-build/bin/python2.5

import sys
import scipy, scipy.optimize, scipy.io, numpy
import re
import FourDAnalysis as fda

if len(sys.argv) < 4:
    sys.exit()

Path          = 'CurveFittingKetyModel.py'
AifCsvFile    = sys.argv[1]
TargetCsvFile = sys.argv[2]
OutputCsvFile = sys.argv[3]

aifCurve      = scipy.io.read_array(AifCsvFile, separator=',')
targetCurve    = scipy.io.read_array(TargetCsvFile, separator=',')
outputCurve   = targetCurve

inputCurveDict = {'AIF': aifCurve}
#initialParamDict = { 'Ktrans': 0.04,
#                     'vp':     0.1,
#                     've':     0.4}

initialParamDict = { 'Ktrans': 0.1,
                     'vp':     0.01,
                     've':     0.01}

print inputCurveDict['AIF']

print targetCurve

caexec = fda.CurveAnalysisExecuter(Path)
result = caexec.Execute(inputCurveDict, initialParamDict, targetCurve, outputCurve)

scipy.io.write_array(OutputCsvFile, outputCurve, separator=',')

for i, v, in result.iteritems():
    sys.stderr.write(' %5s     : %f\n'  % (i, v) )



