import os
import traceback
import logging
import sys

currentDir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(currentDir)
for fileName in os.listdir(currentDir):
  fileNameNoExtension = os.path.splitext(fileName)[0]
  fileExtension = os.path.splitext(fileName)[1]
  if fileExtension == '.py' and fileNameNoExtension != '__init__':
    importStr = 'from ' + fileNameNoExtension + ' import *'
    try:
      exec(importStr)
    except Exception as e:
      logging.error('Failed to import ' + fileNameNoExtension + ': ' + traceback.format_exc())
