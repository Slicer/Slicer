/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerCoreCommandOptions.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerCorePythonManager.h"
# include "ctkPythonConsole.h"
#endif

// Slicer includes

// MRML includes

// VTK includes

// STD includes


// namespace{
// class qSlicerCoreApplicationTest : public qSlicerCoreApplication
// {
// public:
//   qSlicerCoreApplicationTest():qSlicerCoreApplication(){}
// };
// }

int qSlicerCoreApplicationTest1(int argc, char * argv [] )
{
  qSlicerCoreApplication app(argc, argv);

  qSlicerCoreApplication * aptr = app.application();

  if( aptr != (&app) )
    {
    std::cerr << "Problem with the application() singleton" << std::endl;
    return EXIT_FAILURE;
    }

  if (app.coreIOManager() == nullptr)
    {
    std::cerr << "Problem with coreIOManager()" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCoreIOManager * coreIOManager = new qSlicerCoreIOManager;
  app.setCoreIOManager(coreIOManager);

  qSlicerCoreIOManager * coreIOManager2 = app.coreIOManager();
  if (coreIOManager2 != coreIOManager)
    {
    std::cerr << "Problem with setCoreIOManager()/coreIOManager()" << std::endl;
    return EXIT_FAILURE;
    }

  QSettings * settings = app.userSettings();
  if( settings == nullptr )
    {
    std::cerr << "Problem with settings()" << std::endl;
    return EXIT_FAILURE;
    }

  if (app.coreCommandOptions() == nullptr)
    {
    std::cerr << "Problem with coreCommandOptions()" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCoreCommandOptions * coreCommandOptions = new qSlicerCoreCommandOptions;
  app.setCoreCommandOptions(coreCommandOptions);

  qSlicerCoreCommandOptions * coreCommandOptions2 = app.coreCommandOptions();
  if (coreCommandOptions2 != coreCommandOptions)
    {
    std::cerr << "Problem with setCoreCommandOptions()/coreCommandOptions()" << std::endl;
    return EXIT_FAILURE;
    }

  /// Application shouldn't have ask for exit
  if (app.returnCode() != -1)
    {
    std::cerr << "Problem with the application::parseArguments function."
              << "Return code: " << app.returnCode() << std::endl;
    return EXIT_FAILURE;
    }

  bool isInstalled = app.isInstalled();
  if (isInstalled)
    {
    std::cerr << "Problem with isInstalled()" << std::endl;
    return EXIT_FAILURE;
    }

  // Since initialize has been called, the module manager should be available
  qSlicerModuleManager * moduleManager1 = app.moduleManager();

  if( !moduleManager1 )
    {
    std::cerr << "Problem with moduleManager()" << std::endl;
    return EXIT_FAILURE;
    }

  QString homeDirectory = app.slicerHome();

  if (homeDirectory.isEmpty())
    {
    std::cerr << "Problem with slicerHome()" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Slicer Home Directory = " << qPrintable( homeDirectory ) << std::endl;

  vtkSlicerApplicationLogic * logic1 = app.applicationLogic();

  if( logic1 == nullptr )
    {
    std::cerr << "Error in appLogic() " << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLScene * scene1 = app.mrmlScene();

  if( scene1 == nullptr )
    {
    std::cerr << "Error in mrmlScene() " << std::endl;
    return EXIT_FAILURE;
    }

#ifdef Slicer_USE_PYTHONQT

  qSlicerCorePythonManager * pythonManager = app.corePythonManager();
  if (pythonManager)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with  corePythonManager()"
              << " - nullptr pointer is expected." << std::endl;
    return EXIT_FAILURE;
    }

  // Note: qSlicerCoreApplication class takes ownership of the pythonManager and
  // will be responsible to delete it
  app.setCorePythonManager(new qSlicerCorePythonManager());

  pythonManager = app.corePythonManager();
  if (!pythonManager)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with corePythonManager()"
              << " - Return a nullptr pointer." << std::endl;
    return EXIT_FAILURE;
    }

  QObject * foo = new QObject(&app);
  foo->setProperty("something", QVariant(7));
  pythonManager->addObjectToPythonMain("foo", foo);
  pythonManager->executeString("value = foo.something");
  if (pythonManager->getVariable("value").toInt() != 7)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with getVariable()" << std::endl;
    return EXIT_FAILURE;
    }

  ctkPythonConsole * pythonConsole = app.pythonConsole();
  if (pythonConsole)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with  pythonConsole()"
              << " - nullptr pointer is expected." << std::endl;
    return EXIT_FAILURE;
    }

  // Note: qSlicerCoreApplication class takes ownership of the pythonConsole and
  // will be responsible to delete it
  app.setPythonConsole(new ctkPythonConsole());

  pythonConsole = app.pythonConsole();
  if (!pythonConsole)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with pythonConsole()"
              << " - Return a nullptr pointer." << std::endl;
    return EXIT_FAILURE;
    }

#endif

  std::cout << "TEST PASSED !" << std::endl;

  return EXIT_SUCCESS;
}

