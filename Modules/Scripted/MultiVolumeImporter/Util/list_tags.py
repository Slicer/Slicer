from __future__ import print_function
import dicom, string, os, sys

dirIn = sys.argv[1]

files = os.listdir(dirIn)
acqTimes = {}
trigTimes = {}
contTimes = {}
serTimes = {}
tempPositions = {}
for f in files:
  if f.find('.dcm')<=0:
    continue
  dcm = dicom.read_file(dirIn+'/'+f)
  trigTime = dcm.TriggerTime
  acqTime = dcm.AcquisitionTime
  contTime = dcm.ContentTime
  serTime = dcm.SeriesTime
  tempPosition = dcm.TemporalPositionIdentifier

  try:
    trigTimes[trigTime]=trigTimes[trigTime]+1
  except:
    trigTimes[trigTime]=1
  try:
    acqTimes[acqTime]=acqTimes[acqTime]+1
  except:
    acqTimes[acqTime]=1

  try:
    contTimes[contTime]=contTimes[contTime]+1
  except:
    contTimes[contTime]=1
  try:
    serTimes[serTime]=serTimes[serTime]+1
  except:
    serTimes[serTime]=1

  try:
    tempPositions[tempPosition]=tempPositions[tempPosition]+1
  except:
    tempPositions[tempPosition]=1
print('Trigger times: ',trigTimes)
print('Acquisition times: ',acqTimes)
print('Conent times: ',contTimes)
print('Series times: ',serTimes)
print('Temporal positions: ', tempPositions)
