from __future__ import print_function
import time

def TestROIAdd(renameFlag=1, visibilityFlag=1, numToAdd=20):
  print("numToAdd = ", numToAdd)
  if renameFlag > 0:
    print("Index\tTime to add roi\tDelta between adds\tTime to rename roi\tDelta between renames")
    print("%(index)04s\t" % {'index': "i"}, "t\tdt\tt\tdt")
  else:
    print("Index\tTime to add roi\tDelta between adds")
    print("%(index)04s\t" % {'index': "i"}, "t\tdt")
  cx = 0
  cy = 0
  cz = 0
  rx = 1
  ry = 1
  rz = 1
  t1 = 0
  t2 = 0
  t3 = 0
  t4 = 0
  timeToAddThisROI = 0
  timeToAddLastROI = 0
  timeToRenameThisROI = 0
  timeToRenameLastROI = 0
  # iterate over the number of rois to add
  for i in range(numToAdd):
#    print "i = ", i, "/", numToAdd, ", r = ", r, ", a = ", a, ", s = ", s
    roiNode = slicer.vtkMRMLAnnotationROINode()
    roiNode.SetXYZ(cx, cy, cz)
    roiNode.SetRadiusXYZ(rx, ry, rz)
    t1 = time.clock()
    roiNode.Initialize(slicer.mrmlScene)
    t2 = time.clock()
    timeToAddThisROI = t2 - t1
    dt = timeToAddThisROI - timeToAddLastROI
    if renameFlag > 0:
      t3 = time.clock()
      roiNode.SetName(str(i))
      t4 = time.clock()
      timeToRenameThisROI = t4 - t3
      dt2 = timeToRenameThisROI - timeToRenameLastROI
      print('%(index)04d\t' % {'index': i}, timeToAddThisROI, "\t", dt, "\t", timeToRenameThisROI, "\t", dt2)
      timeToRenameLastROI = timeToRenameThisROI
    else:
      print('%(index)04d\t' % {'index': i}, timeToAddThisROI, "\t", dt)
    rx = rx + 0.5
    ry = ry + 0.5
    rz = rz + 0.5
    cx = cx + 2.0
    cy = cy + 2.0
    cz = cz + 2.0
    timeToAddLastROI = timeToAddThisROI

testStartTime = time.clock()
TestROIAdd()
testEndTime = time.clock()
testTime = testEndTime - testStartTime
print("Test total time = ", testTime)

