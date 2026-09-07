import sys
import unittest

import vtk
import slicer


class SlicerLazyVtkModuleTest(unittest.TestCase):
    """Check that objects returned by C++ methods are wrapped as their actual class,
    even if the VTK module that provides that class has not been imported yet.

    Slicer loads the VTK Python modules lazily: only the modules that provide base
    classes of the wrapped Slicer classes are imported at startup. When a C++ method
    returns an object of a class that has not been imported yet, the wrapping must
    import the module that provides it, otherwise the object would be returned as its
    nearest already-imported base class and would lack the methods of the actual class
    (see https://github.com/Slicer/Slicer/issues/9382).
    """

    def test_lazyLoadingIsEnabled(self):
        # Sanity check: if every VTK module were already loaded then this test would not test anything.
        self.assertNotIn("vtkmodules.vtkChartsCore", sys.modules)

    def test_plotViewChartLegend(self):
        # Reproduces https://github.com/Slicer/Slicer/issues/9382:
        # vtkChartsCore is not a base class provider of any Slicer class, so it is not imported at startup.
        layoutManager = slicer.app.layoutManager()
        layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpPlotView)
        chart = layoutManager.plotWidget(0).plotView().chart()
        # Without the fix the chart is returned as a vtkContextItem, which has no GetLegend method
        self.assertEqual(type(chart).__name__, chart.GetClassName())
        self.assertEqual(type(chart).__name__, "vtkChartXY")
        legend = chart.GetLegend()
        self.assertEqual(type(legend).__name__, "vtkChartLegend")
        self.assertEqual(type(legend).__module__, "vtkmodules.vtkChartsCore")
        # The lazily imported class is the one that the vtk module provides
        self.assertIs(type(legend), vtk.vtkChartLegend)
        self.assertIsInstance(legend, vtk.vtkContextItem)

    def test_objectFactoryOverride(self):
        # vtkRenderer::New() returns a vtkOpenGLRenderer (from vtkRenderingOpenGL2), which
        # is not a base class of any Slicer class, so it is not imported at startup.
        renderer = vtk.vtkRenderer()
        self.assertEqual(type(renderer).__name__, renderer.GetClassName())
        self.assertEqual(type(renderer).__module__, "vtkmodules.vtkRenderingOpenGL2")
        self.assertIsInstance(renderer, vtk.vtkRenderer)
