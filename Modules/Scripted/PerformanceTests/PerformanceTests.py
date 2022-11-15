import qt
import slicer

from slicer.ScriptedLoadableModule import *


#
# PerformanceTests
#

class PerformanceTests(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "Performance Tests"
        parent.categories = ["Testing.TestCases"]
        parent.contributors = ["Steve Pieper (Isomics)"]
        parent.helpText = """
    Module to run interactive performance tests on the core of slicer.
    """
        parent.acknowledgementText = """
    This file was based on work originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
and others.  This work was partially funded by NIH grant 3P41RR013218-12S1.
    """
        self.parent = parent


#
# qPerformanceTestsWidget
#

class PerformanceTestsWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        tests = (
            ('Get Sample Data', self.downloadMRHead),
            ('Reslicing', self.reslicing),
            ('Crosshair Jump', self.crosshairJump),
            ('Memory Check', self.memoryCheck),
        )

        for test in tests:
            b = qt.QPushButton(test[0])
            self.layout.addWidget(b)
            b.connect('clicked()', test[1])

        self.log = qt.QTextEdit()
        self.log.readOnly = True
        self.layout.addWidget(self.log)
        self.log.insertHtml('<p>Status: <i>Idle</i>\n')
        self.log.insertPlainText('\n')
        self.log.ensureCursorVisible()

        # Add spacer to layout
        self.layout.addStretch(1)

    def downloadMRHead(self):
        import SampleData
        self.log.insertHtml('<b>Requesting downloading MRHead')
        self.log.repaint()
        mrHeadVolume = SampleData.downloadSample("MRHead")
        if mrHeadVolume:
            self.log.insertHtml('<i>finished.</i>\n')
            self.log.insertPlainText('\n')
            self.log.repaint()
        else:
            self.log.insertHtml('<b>Download failed!</b>\n')
            self.log.insertPlainText('\n')
            self.log.repaint()
        self.log.ensureCursorVisible()

    def timeSteps(self, iters, f):
        import time
        elapsedTime = 0
        for i in range(iters):
            startTime = time.time()
            f()
            slicer.app.processEvents()
            endTime = time.time()
            elapsedTime += (endTime - startTime)
        fps = int(iters / elapsedTime)
        result = f"fps = {fps:g} ({1000./fps:g} ms per frame)"
        print(result)
        self.log.insertHtml('<i>%s</i>' % result)
        self.log.insertPlainText('\n')
        self.log.ensureCursorVisible()
        self.log.repaint()

    def reslicing(self, iters=100):
        """ go into a loop that stresses the reslice performance
        """
        import time
        import math
        import numpy as np
        sliceNode = slicer.util.getNode('vtkMRMLSliceNodeRed')
        dims = sliceNode.GetDimensions()
        elapsedTime = 0
        sliceOffset = 5
        offsetSteps = 10
        numerOfSweeps = int(math.ceil(iters / offsetSteps))
        renderingTimesSec = np.zeros(numerOfSweeps * offsetSteps * 2)
        sampleIndex = 0
        startOffset = sliceNode.GetSliceOffset()
        for i in range(numerOfSweeps):
            for offset in ([sliceOffset] * offsetSteps + [-sliceOffset] * offsetSteps):
                startTime = time.time()
                sliceNode.SetSliceOffset(sliceNode.GetSliceOffset() + offset)
                slicer.app.processEvents()
                endTime = time.time()
                renderingTimesSec[sampleIndex] = (endTime - startTime)
                sampleIndex += 1
        sliceNode.SetSliceOffset(startOffset)

        resultTableName = slicer.mrmlScene.GetUniqueNameByString("Reslice performance")
        resultTableNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLTableNode", resultTableName)
        slicer.util.updateTableFromArray(resultTableNode, renderingTimesSec, "Rendering time [s]")

        renderingTimeMean = np.mean(renderingTimesSec)
        renderingTimeStd = np.std(renderingTimesSec)
        result = ("%d x %d, fps = %.1f (%.1f +/- %.2f ms per frame) - see details in table '%s'"
                  % (dims[0], dims[1], 1.0 / renderingTimeMean, 1000. * renderingTimeMean, 1000. * renderingTimeStd, resultTableNode.GetName()))
        print(result)
        self.log.insertHtml('<i>%s</i>' % result)
        self.log.insertPlainText('\n')
        self.log.ensureCursorVisible()
        self.log.repaint()

    def crosshairJump(self, iters=15):
        """ go into a loop that stresses jumping to slices by moving crosshair
        """
        import time
        sliceNode = slicer.util.getNode('vtkMRMLSliceNodeRed')
        dims = sliceNode.GetDimensions()
        layoutManager = slicer.app.layoutManager()
        sliceViewNames = layoutManager.sliceViewNames()
        # Order of slice view names is random, prefer 'Red' slice to make results more predictable
        firstSliceViewName = 'Red' if 'Red' in sliceViewNames else sliceViewNames[0]
        firstSliceWidget = layoutManager.sliceWidget(firstSliceViewName)
        elapsedTime = 0
        startPoint = (int(dims[0] * 0.3), int(dims[1] * 0.3))
        endPoint = (int(dims[0] * 0.6), int(dims[1] * 0.6))
        for i in range(iters):
            startTime = time.time()
            slicer.util.clickAndDrag(firstSliceWidget, button=None, modifiers=['Shift'], start=startPoint, end=endPoint, steps=2)
            slicer.app.processEvents()
            endTime1 = time.time()
            slicer.util.clickAndDrag(firstSliceWidget, button=None, modifiers=['Shift'], start=endPoint, end=startPoint, steps=2)
            slicer.app.processEvents()
            endTime2 = time.time()
            delta = ((endTime1 - startTime) + (endTime2 - endTime1)) / 2.
            elapsedTime += delta
        fps = int(iters / elapsedTime)
        result = "number of slice views = %d, fps = %g (%g ms per frame)" % (len(sliceViewNames), fps, 1000. / fps)
        print(result)
        self.log.insertHtml('<i>%s</i>' % result)
        self.log.insertPlainText('\n')
        self.log.ensureCursorVisible()
        self.log.repaint()

    def memoryCallback(self):
        if self.sysInfoWindow.visible:
            self.sysInfo.RunMemoryCheck()
            self.sysInfoWindow.append('p: %d of %d,  v: %d of %d' %
                                      (self.sysInfo.GetAvailablePhysicalMemory(),
                                       self.sysInfo.GetTotalPhysicalMemory(),
                                       self.sysInfo.GetAvailableVirtualMemory(),
                                       self.sysInfo.GetTotalVirtualMemory(),
                                       ))
            qt.QTimer.singleShot(1000, self.memoryCallback)

    def memoryCheck(self):
        """Run a periodic memory check in a window"""
        if not hasattr(self, 'sysInfo'):
            self.sysInfo = slicer.vtkSystemInformation()
            self.sysInfoWindow = qt.QTextBrowser()
        if self.sysInfoWindow.visible:
            return
        self.sysInfoWindow.show()
        self.memoryCallback()


class sliceLogicTest:
    def __init__(self):
        self.step = 0
        self.sliceLogic = slicer.vtkMRMLSliceLayerLogic()
        self.sliceLogic.SetMRMLScene(slicer.mrmlScene)
        self.sliceNode = slicer.vtkMRMLSliceNode()
        self.sliceNode.SetLayoutName("Black")
        slicer.mrmlScene.AddNode(self.sliceNode)
        self.sliceLogic.SetSliceNode(self.sliceNode)

    def stepSliceLogic(self):
        self.sliceNode.SetSliceOffset(-1 * self.step * 10)
        self.step = 1 ^ self.step

    def testSliceLogic(self, iters):
        timeSteps(iters, self.stepSliceLogic)
