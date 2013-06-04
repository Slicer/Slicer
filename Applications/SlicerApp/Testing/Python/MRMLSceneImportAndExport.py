import slicer
import os

def testMRMLSceneImportAndExport():
  tempDir = slicer.app.temporaryPath
  scenePath = tempDir + '/temp_scene.mrml'
  slicer.mrmlScene.SetURL(scenePath)
  if not slicer.mrmlScene.Commit(scenePath):
    raise Exception('Saving a MRML scene failed !')

  success = slicer.mrmlScene.Import()
  os.remove(scenePath)
  if not success:
    raise Exception('Importing back a MRML scene failed !')

if __name__ == '__main__':
  testMRMLSceneImportAndExport()
