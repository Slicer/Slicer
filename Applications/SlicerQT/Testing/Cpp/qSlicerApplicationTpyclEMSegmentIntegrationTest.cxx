
// CTK includes
#include <ctkScopedCurrentDir.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCorePythonManager.h"

// EMSegment includes
#include "vtkSlicerCommonInterface.h"

// STD includes
#include <cstdio>

// test the tcl adapter provided by the Slicer Common Interface
int qSlicerApplicationTpyclEMSegmentIntegrationTest(int argc, char* argv[])
{
  if (argc < 2)
    {
    std::cerr << "Usage:\n"
              << "  " << argv[0] << " </path/to/EMSegmentCommandLine>" << std::endl;
    return EXIT_FAILURE;
    }

  vtkSlicerCommonInterface* common = vtkSlicerCommonInterface::New();

  // Slicer4
  qSlicerApplication* app = new qSlicerApplication(argc, argv);

#ifdef Slicer_USE_PYTHONQT_WITH_TCL
  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);
  if(!disablePython)
    {
    // Load EMSegment Python module(s)
    QString emsegmentModulePath = QString::fromStdString(
          vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory(argv[1]));
    emsegmentModulePath.append("/" Slicer_QTLOADABLEMODULES_SUBDIR);
    app->pythonManager()->appendPythonPath(emsegmentModulePath);
    // Update current application directory, so that *PythonD modules can be loaded
    ctkScopedCurrentDir scopedCurrentDir(emsegmentModulePath);

    QString emsegmentModulePythonPath = emsegmentModulePath + "/Python";
    std::cout << "emsegmentModulePythonPath:" << qPrintable(emsegmentModulePythonPath) << std::endl;
    app->pythonManager()->executeString(QString(
      "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer%2ModuleLogic.py');"
      "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer%2ModuleMRML.py');"
      ).arg(emsegmentModulePythonPath).arg("EMSegment"));

    }
#endif

  delete app;

  common->Delete();

  return EXIT_SUCCESS;
}
