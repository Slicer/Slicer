import os

# set the url
scenePath =  os.path.join(os.environ['SLICER_HOME'], "../../Slicer4/Modules/Loadable/Annotations/Testing/Data/Input/ruler.mrml")
scenePath = os.path.normpath(scenePath)
print "Trying to load ruler mrml file", scenePath
slicer.mrmlScene.SetURL(scenePath)


# and load it
slicer.mrmlScene.Connect()

