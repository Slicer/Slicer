import os
for fileName in os.listdir(os.path.dirname(os.path.realpath(__file__))):
  fileNameNoExtension = os.path.splitext(fileName)[0]
  fileExtension = os.path.splitext(fileName)[1]
  if fileExtension == '.py' and fileNameNoExtension != '__init__':
    importStr = 'from ' + fileNameNoExtension + ' import *'
    exec(importStr)
