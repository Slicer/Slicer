
import os
import unittest
import vtk
import qt
import slicer
import EditorLib

class SlicerMRB(unittest.TestCase):
  """ Test for slicer data bundle

Run manually from within slicer by pasting an version of this with the correct path into the python console:
execfile('/Users/pieper/slicer4/latest/Slicer/Applications/SlicerApp/Testing/Python/SlicerMRBTest.py'); t = SlicerMRB(); t.setUp(); t.runTest()

  """

  def __init__(self,methodName='runTest', useCase='big',uniqueDirectory=True,strict=False):
    """
    Tests the use of mrml and mrb save formats with volumes and fiber bundles.
    Checks that scene views are saved and restored as expected.

    useCase: string defining what scene to create
             'small' for one volume and tract
             'big', 2 volumes, 3 tracts
    uniqueDirectory: boolean about save directory
                     False to reuse standard dir name
                     True timestamps dir name
    strict: boolean about how carefully to check result
                     True then check every detail
                     False then confirm basic operation, but allow non-critical issues to pass
    """
    unittest.TestCase.__init__(self,methodName)
    self.useCase = useCase
    self.uniqueDirectory = uniqueDirectory
    self.strict = strict

  def delayDisplay(self,message,msec=1000):
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    self.setUp()
    self.test_SlicerMRB()
    self.test_PercentEncode()


  def test_SlicerMRB(self):
    """
    Replicate the issue reported in bug 2385 where saving
    and restoring an MRB file does not work.
    """

    print("Running SlicerMRB Test case with:")
    print("useCase : %s" % self.useCase)
    print("uniqueDirectory : %s" % self.uniqueDirectory)
    print("strict : %s" % self.strict)

    #
    # first, get the data
    # - amount of data depends on useCase attribue
    #
    import urllib
    if self.useCase == 'small':
      downloads = (
          ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
          ('http://slicer.kitware.com/midas3/download?items=5768', 'tract1.vtk', slicer.util.loadFiberBundle),
          )
      tracts = ('tract1',)
      tractColors = ( (0.2, 0.9, 0.3), )
    elif self.useCase == 'big':
      downloads = (
          ('http://slicer.kitware.com/midas3/download?items=5766', 'DTIVolume.raw.gz', None),
          ('http://slicer.kitware.com/midas3/download?items=5765', 'DTIVolume.nhdr', slicer.util.loadVolume),
          ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
          ('http://slicer.kitware.com/midas3/download?items=5768', 'tract1.vtk', slicer.util.loadFiberBundle),
          ('http://slicer.kitware.com/midas3/download?items=5769', 'tract2.vtk', slicer.util.loadFiberBundle),
          ('http://slicer.kitware.com/midas3/download?items=5770', 'tract3.vtk', slicer.util.loadFiberBundle),
          )
      tracts = ('tract1', 'tract2', 'tract3',)
      tractColors = ( (0.2, 0.9, 0.3), (0.9, 0.3, 0.3), (0.2, 0.4, 0.9) )

    # perform the downloads if needed, then load
    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        self.delayDisplay('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        self.delayDisplay('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    # confirm that FA is in the background of the Red slice
    redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
    fa = slicer.util.getNode('FA')
    self.assertTrue( redComposite.GetBackgroundVolumeID() == fa.GetID() )
    self.delayDisplay('The FA volume is in the background of the Red viewer')

    #
    # set up the view of the tracts and scene views
    #
    for tractName,tubeColor in zip(tracts, tractColors):
      fiberNode = slicer.util.getNode(tractName)
      tubeDisplay = fiberNode.GetTubeDisplayNode()
      tubeDisplay.SetColor(tubeColor)
      tubeDisplay.SetColorModeToSolid()

    # turn on one at a time and save scene view
    for tractName in tracts:
      self.showOneTract(tracts,tractName)
      self.delayDisplay('Showing %s' % tractName)
      self.storeSceneView('%s-view' % tractName, "Only show tubes for %s" % tractName)

    #
    # save the mrml scene to a temp directory, then zip it
    #
    applicationLogic = slicer.app.applicationLogic()
    sceneSaveDirectory = self.tempDirectory('__scene__')
    mrbFilePath= self.tempDirectory('__mrb__') + '/test.mrb'
    self.delayDisplay("Saving scene to: %s\n" % sceneSaveDirectory + "Saving mrb to: %s" % mrbFilePath)
    self.assertTrue(
        applicationLogic.SaveSceneToSlicerDataBundleDirectory(sceneSaveDirectory, None)
    )
    self.delayDisplay("Finished saving scene")
    self.assertTrue(
        applicationLogic.Zip(mrbFilePath,sceneSaveDirectory)
    )
    self.delayDisplay("Finished saving MRB")

    # confirm again that FA is in the background of the Red slice
    self.delayDisplay('Check that FA volume is STILL in the background of the Red viewer')
    redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
    fa = slicer.util.getNode('FA')
    self.assertTrue( redComposite.GetBackgroundVolumeID() == fa.GetID() )
    self.delayDisplay('The FA volume is STILL in the background of the Red viewer')

    #
    # save image, reload the scene and save image
    # then load mrb and save image
    # - all images should be the same
    #
    self.delayDisplay('Save and image, then close scene')
    self.beforeImage = qt.QPixmap.grabWidget(slicer.util.mainWindow()).toImage()
    slicer.mrmlScene.Clear(0)
    self.delayDisplay('Now, reload the saved scene')
    slicer.mrmlScene.SetURL(sceneSaveDirectory + '/' + os.path.basename(sceneSaveDirectory) + '.mrml')
    self.assertTrue(
        slicer.mrmlScene.Import()
    )
    slicer.app.processEvents()

    # confirm again that FA is in the background of the Red slice
    self.delayDisplay('IS the FA volume is AGAIN in the background of the Red viewer?')
    redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
    fa = slicer.util.getNode('FA')
    self.assertTrue( redComposite.GetBackgroundVolumeID() == fa.GetID() )
    self.delayDisplay('The FA volume is AGAIN in the background of the Red viewer')

    self.sceneLoadedImage = qt.QPixmap.grabWidget(slicer.util.mainWindow()).toImage()
    slicer.mrmlScene.Clear(0)
    mrbExtractPath = self.tempDirectory('__mrb_extract__')
    mrbLoaded = applicationLogic.OpenSlicerDataBundle(mrbFilePath, mrbExtractPath)
    # load can return false even though it succeeded - only fail if in strict mode
    self.assertTrue( not self.strict or mrbLoaded )
    slicer.app.processEvents()
    self.mrbLoadedImage = qt.QPixmap.grabWidget(slicer.util.mainWindow()).toImage()

    # confirm that FA is in the background of the Red slice after mrb reload
    self.delayDisplay('FA volume is the background of the Red viewer after mrb reload?')
    redComposite = slicer.util.getNode('vtkMRMLSliceCompositeNodeRed')
    fa = slicer.util.getNode('FA')
    self.assertTrue( redComposite.GetBackgroundVolumeID() == fa.GetID() )
    self.delayDisplay('Yes, the FA volume is back in the background of the Red viewer')

    sceneMismatch = self.beforeImage != self.sceneLoadedImage
    mrbMismatch = self.beforeImage != self.mrbLoadedImage
    if sceneMismatch:
      self.delayDisplay('Loaded scene image does not match original scene image')
    if mrbMismatch:
      self.delayDisplay('Loaded mrb image does not match original scene image')

    if (sceneMismatch or mrbMismatch):
      self.imageCompare((self.beforeImage, self.sceneLoadedImage, self.mrbLoadedImage), "before, after scene load, and after mrb load")
      self.delayDisplay('have a quick look...', msec=2000)
      imagesAndNames = ((self.beforeImage,'before'), (self.sceneLoadedImage,'scene'), (self.mrbLoadedImage,'mrb'))
      for i,name in imagesAndNames:
        tmp = self.tempDirectory('')
        filePath = tmp + '/' + name + '.png'
        print("Saving image to %s" % filePath)
        i.save(filePath)

    # images can differe due to widget size and rendering issues - only fail if in strict mode
    self.assertTrue( not self.strict or (not sceneMismatch and not mrbMismatch) )

    self.delayDisplay("Scene and MRB loaded and compared")

    for tractName,tubeColor in zip(tracts, tractColors):
      sceneView = slicer.util.getNode('%s-view' % tractName)
      sceneView.RestoreScene()
      self.delayDisplay("Should now see tubes for tract %s in color %s" % (tractName, str(tractColors)))

    self.delayDisplay("Test Finished")

  def test_PercentEncode(self):
    """ Replicate the issue reported in bug 2605 where saving
    and MRB file that includes DICOM values doesn't work
    on windows.  In this case, the name of the volume includes
    a ':' character, and is interpreted as a drive letter on windows.
    TODO: there should be a Cxx level application logic test too.
    """
    appLogic = slicer.app.applicationLogic()
    stringPairs = (
        ('', ''),
        ('test', 'test'),
        ('tEstAZ', 'tEstAZ'),
        ('t#$%^&()<>{}[]EstAZ', 't#$%^&()<>{}[]EstAZ'),
        ('test./:', 'test.%2f%3a'),
      )
    for original,encoded in stringPairs:
      self.delayDisplay("Testing that %s becomes %s" % (original,encoded), 150)
      print ('%s becomes %s, should be %s' % (original, appLogic.PercentEncode(original), encoded))
      self.assertTrue( appLogic.PercentEncode(original) == encoded )
    self.delayDisplay("Test Finished")

  def imageCompare(self,images,text='',prefWidth=1500):
    """Show images in a window with a text message.
    """
    self.imageCompare = qt.QWidget()
    vbox = qt.QVBoxLayout()
    self.imageCompare.setLayout(vbox)
    label = qt.QLabel(self.imageCompare)
    label.text = text
    vbox.addWidget(label)
    imageFrame = qt.QFrame(self.imageCompare)
    vbox.addWidget(imageFrame)
    hbox = qt.QHBoxLayout()
    imageFrame.setLayout(hbox)
    for image in images:
      ii = image.scaledToWidth(prefWidth/len(images))
      label = qt.QLabel(imageFrame)
      label.pixmap = label.pixmap.fromImage(ii)
      hbox.addWidget(label)
    self.imageCompare.show()

  def showOneTract(self,tracts,whichTract):
    """display the named tract's tubes but turn the others off"""
    for tractName in tracts:
      fiberNode = slicer.util.getNode(tractName)
      tubeDisplay = fiberNode.GetTubeDisplayNode()
      if tractName == whichTract:
        tubeDisplay.SetVisibility(1)
        tubeDisplay.SetSliceIntersectionVisibility(1)
      else:
        tubeDisplay.SetVisibility(0)
        tubeDisplay.SetSliceIntersectionVisibility(0)

  def storeSceneView(self,name,description=""):
    """  Store a scene view into the current scene.
    TODO: this might move to slicer.util
    """
    layoutManager = slicer.app.layoutManager()

    sceneViewNode = slicer.vtkMRMLSceneViewNode()
    view1 = layoutManager.threeDWidget(0).threeDView()

    w2i1 = vtk.vtkWindowToImageFilter()
    w2i1.SetInput(view1.renderWindow())

    w2i1.Update()
    image1 = w2i1.GetOutput()
    sceneViewNode.SetScreenShot(image1)
    sceneViewNode.UpdateStoredScene()
    slicer.mrmlScene.AddNode(sceneViewNode)

    sceneViewNode.SetName(name)
    sceneViewNode.SetSceneViewDescription(description)
    sceneViewNode.StoreScene()

    return sceneViewNode

  def tempDirectory(self,key='__SlicerTestTemp__',tempDir=None):
    """Come up with a unique directory name in the temp dir and make it and return it
    # TODO: switch to QTemporaryDir in Qt5.
    # For now, create a named directory if uniqueDirectory attribute is true
    Note: this directory is not automatically cleaned up
    """
    if not tempDir:
      tempDir = qt.QDir(slicer.app.temporaryPath)
    tempDirName = key
    if self.uniqueDirectory:
      key += qt.QDateTime().currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz")
    fileInfo = qt.QFileInfo(qt.QDir(tempDir), tempDirName)
    dirPath = fileInfo.absoluteFilePath()
    qt.QDir().mkpath(dirPath)
    return dirPath


#
# SlicerMRBTest
#

class SlicerMRBTest:
  """
  This class is the 'hook' for slicer to detect and recognize the test
  as a loadable scripted module (with a hidden interface)
  """
  def __init__(self, parent):
    parent.title = "SlicerMRBTest"
    parent.categories = ["Testing"]
    parent.contributors = ["Steve Pieper (Isomics Inc.)"]
    parent.helpText = """
    Self test for the editor.
    No module interface here, only used in SelfTests module
    """
    parent.acknowledgementText = """
    This DICOM Plugin was developed by
    Steve Pieper, Isomics, Inc.
    and was partially funded by NIH grant 3P41RR013218.
    """

    # don't show this module
    parent.hidden = True

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['SlicerMRBTest'] = self.runTest

  def runTest(self):
    tester = SlicerMRB()
    tester.setUp()
    tester.runTest()


#
# SlicerMRBTestWidget
#

class SlicerMRBTestWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass


