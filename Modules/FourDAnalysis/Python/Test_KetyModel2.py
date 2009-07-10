#! /Users/junichi/igtdev/slicer/Slicer3-lib/python-build/bin/python2.5
###! /opt/local/bin/python2.6


#from multiprocessing import Queue, Process, Manager
import sys
import scipy, scipy.optimize, scipy.io, numpy
import re
import FourDAnalysis as fda

if len(sys.argv) < 4:
    sys.exit()

Path          = 'CurveFittingKetyModel2.py'
AifCsvFile    = sys.argv[1]
TargetCsvFile = sys.argv[2]
OutputCsvFile = sys.argv[3]

aifCurve      = scipy.io.read_array(AifCsvFile, separator=',')
targetCurve   = scipy.io.read_array(TargetCsvFile, separator=',')
outputCurve   = targetCurve

initialParamDict = { 'Ktrans': 0.1,
                     've':     0.1}

inputCurveDict = {'AIF': aifCurve}

inputParamDict = {}

caexec = fda.CurveAnalysisExecuter(Path)

# Single process
result = caexec.Execute(inputCurveDict, initialParamDict, inputParamDict, targetCurve, outputCurve)

## multiprocess
#q = Queue()
#
#dict = {}
#dict['inputCurveDict']   = inputCurveDict
#dict['initialParameterDict'] = initialParamDict
#dict['inputParameterDict']   = inputParamDict
#dict['targetCurve']          = targetCurve
#dict['outputCurve']          = outputCurve
#
#q.put(dict)
#
#p = Process(target = caexec.ExecuteWithQueue, args=(q,))
#
#p.start()
#
#p.join()
#
#rdict       = q.get()
#result      = rdict['result']
#outputCurve = rdict['outputCurve']

scipy.io.write_array(OutputCsvFile, outputCurve, separator=',')

for i, v, in result.iteritems():
    sys.stderr.write(' %5s     : %f\n'  % (i, v) )




