from __future__ import print_function
import os

# try get the path of the ruler scene file from the arguments
numArgs = len(sys.argv)
if numArgs > 1:
  scenePath = sys.argv[1]
else:
  # set the url as best guess from SLICER_HOME
  scenePath =  os.path.join(os.environ['SLICER_HOME'], "../../Slicer4/Modules/Loadable/Annotations/Testing/Data/Input/ruler.mrml")

scenePath = os.path.normpath(scenePath)
print("Trying to load ruler mrml file", scenePath)
slicer.mrmlScene.SetURL(scenePath)


# and load it
slicer.mrmlScene.Connect()

