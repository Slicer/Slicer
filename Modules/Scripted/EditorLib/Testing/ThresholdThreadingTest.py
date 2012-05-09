

comment = """
import os

src = os.environ['HOME'] + '/slicer4/latest/Slicer/Modules/Scripted/EditorLib/Testing/ThresholdThreadingTest.py'
execfile(src)
thresholdThreadingTest()

"""

def thresholdThreadingTest():
  print('running test!')

