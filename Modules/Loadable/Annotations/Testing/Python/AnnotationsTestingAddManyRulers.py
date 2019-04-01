from __future__ import print_function
import time

def TestRulerAdd(renameFlag=1, visibilityFlag=1, numToAdd=20):
  print("numToAdd = ", numToAdd)
  if renameFlag > 0:
    print("Index\tTime to add ruler\tDelta between adds\tTime to rename ruler\tDelta between renames")
    print("%(index)04s\t" % {'index': "i"}, "t\tdt\tt\tdt")
  else:
    print("Index\tTime to add ruler\tDelta between adds")
    print("%(index)04s\t" % {'index': "i"}, "t\tdt")
  r1 = 0
  a1 = 0
  s1 = 0
  r2 = 1
  a2 = 1
  s2 = 1
  t1 = 0
  t2 = 0
  t3 = 0
  t4 = 0
  timeToAddThisRuler = 0
  timeToAddLastRuler = 0
  timeToRenameThisRuler = 0
  timeToRenameLastRuler = 0
  # iterate over the number of rulers to add
  for i in range(numToAdd):
#    print "i = ", i, "/", numToAdd, ", r = ", r, ", a = ", a, ", s = ", s
    rulerNode = slicer.vtkMRMLAnnotationRulerNode()
    rulerNode.SetPosition1(r1, a1, s1)
    rulerNode.SetPosition2(r2, a2, s2)
    t1 = time.clock()
    rulerNode.Initialize(slicer.mrmlScene)
    t2 = time.clock()
    timeToAddThisRuler = t2 - t1
    dt = timeToAddThisRuler - timeToAddLastRuler
    if renameFlag > 0:
      t3 = time.clock()
      rulerNode.SetName(str(i))
      t4 = time.clock()
      timeToRenameThisRuler = t4 - t3
      dt2 = timeToRenameThisRuler - timeToRenameLastRuler
      print('%(index)04d\t' % {'index': i}, timeToAddThisRuler, "\t", dt, "\t", timeToRenameThisRuler, "\t", dt2)
      timeToRenameLastRuler = timeToRenameThisRuler
    else:
      print('%(index)04d\t' % {'index': i}, timeToAddThisRuler, "\t", dt)
    r1 = r1 + 1.0
    a1 = a1 + 1.0
    s1 = s1 + 1.0
    r2 = r2 + 1.5
    a2 = a2 + 1.5
    s2 = s2 + 1.5
    timeToAddLastRuler = timeToAddThisRuler

testStartTime = time.clock()
TestRulerAdd()
testEndTime = time.clock()
testTime = testEndTime - testStartTime
print("Test total time = ", testTime)

