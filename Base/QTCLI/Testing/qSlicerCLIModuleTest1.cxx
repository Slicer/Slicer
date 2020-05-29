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

// Qt includes
#include <QPushButton>
#include <QTemporaryFile>
#include <QTimer>

// CTK includes
#include <ctkCallback.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleWidget.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"

// MRMLCLI includes
#include <vtkMRMLCommandLineModuleNode.h>
#include <vtkSlicerCLIModuleLogic.h>

// STD includes

namespace
{
//-----------------------------------------------------------------------------
qSlicerCLIModule * CLIModule;
QString            ErrorString;

//-----------------------------------------------------------------------------
void runCli(void * data)
{
  Q_ASSERT(CLIModule);
  Q_UNUSED(data);

  QTemporaryFile outputFile("qSlicerCLIModuleTest1-outputFile-XXXXXX");
  if (!outputFile.open())
    {
    ErrorString = "Failed to create temporary file";
    return;
    }
  //outputFile.close();

  // Create node
  vtkMRMLCommandLineModuleNode * cliModuleNode =
    CLIModule->cliModuleLogic()->CreateNodeInScene();

  // Values
  int inputValue1 = 4;
  int inputValue2 = 3;

  // Set node parameters
  cliModuleNode->SetParameterAsInt("InputValue1", inputValue1);
  cliModuleNode->SetParameterAsInt("InputValue2", inputValue2);
  cliModuleNode->SetParameterAsString("OperationType", "Addition");
  cliModuleNode->SetParameterAsString("OutputFile", outputFile.fileName().toStdString());

  // Execute synchronously so that we can check the content of the file after the module execution
  CLIModule->cliModuleLogic()->ApplyAndWait(cliModuleNode);

  // Read outputFile
  QTextStream stream(&outputFile);
  QString operationResult = stream.readAll().trimmed();

  QString expectedResult = QString::number(inputValue1 + inputValue2);
  if (operationResult.compare(expectedResult) != 0)
    {
    ErrorString = QString("OutputFile doesn't contain the expected result !\n"
                          "\tExpected:%1\n"
                          "\tCurrent:%2").arg(expectedResult).arg(operationResult);
    return;
    }

  outputFile.close();
}

} // end anonymous namespace

//-----------------------------------------------------------------------------
int qSlicerCLIModuleTest1(int argc, char * argv[])
{
  // The CLI4Test module (CLIModule4Test) has already been built as a normal
  // CLI library. It can be found in
  // Slicer-build/lib/Slicer-X.Y/cli-modules[/Debug|Release]
  QString cliModuleName("CLI4Test");

  qSlicerApplication::setAttribute(qSlicerApplication::AA_DisablePython);
  qSlicerApplication app(argc, argv);

  qSlicerModuleManager * moduleManager = app.moduleManager();
  if (!moduleManager)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qSlicerApplication::moduleManager()" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerModuleFactoryManager* moduleFactoryManager = moduleManager->factoryManager();
  if (!moduleFactoryManager)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qSlicerModuleManager::factoryManager()" << std::endl;
    return EXIT_FAILURE;
    }

  moduleFactoryManager->registerFactory(new qSlicerCLILoadableModuleFactory);
  QString cliPath = app.slicerHome() + "/" + Slicer_CLIMODULES_LIB_DIR + "/";
  moduleFactoryManager->addSearchPath(cliPath);
  moduleFactoryManager->addSearchPath(cliPath + app.intDir());

  // Register and instantiate modules
  moduleFactoryManager->registerModules();
  moduleFactoryManager->instantiateModules();

  QStringList moduleNames = moduleFactoryManager->instantiatedModuleNames();
  if (!moduleNames.contains(cliModuleName))
    {
    std::cerr << "Line " << __LINE__ << " - Problem with qSlicerCLILoadableModuleFactory"
              << " - Failed to register '" << qPrintable(cliModuleName) << "' module" << std::endl;
    return EXIT_FAILURE;
    }

  foreach(const QString& name, moduleNames)
    {
    moduleFactoryManager->loadModule(name);
    }

  qSlicerAbstractCoreModule * module = moduleManager->module("CLI4Test");
  if (!module)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qSlicerModuleManager::module()"
              << " - Failed to retrieve module named '" << qPrintable(cliModuleName) << "'" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCLIModule * cliModule = qobject_cast<qSlicerCLIModule*>(module);
  if (!cliModule)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to cast module named '" << qPrintable(cliModuleName) << "' "
              << "from [qSlicerAbstractCoreModule*] into [qSlicerCLIModule*]" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerAbstractModuleRepresentation * widgetRepresentation = cliModule->widgetRepresentation();
  if (!widgetRepresentation)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qSlicerCLIModule::widgetRepresentation()"
              << " - Failed to retrieve representation associated with module named '"
              << qPrintable(cliModuleName) << "'" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCLIModuleWidget* cliWidget =
    dynamic_cast<qSlicerCLIModuleWidget*>(widgetRepresentation);
  if (!cliWidget)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to cast module '" << qPrintable(cliModuleName) << "' representation "
              << "from [qSlicerAbstractModuleRepresentation*] into [qSlicerCLIModuleWidget*]" << std::endl;
    return EXIT_FAILURE;
    }

  cliWidget->show();

  QPushButton button("Simulate CLI programmatic start");
  CLIModule = cliModule;
  ctkCallback callback(runCli);
  QObject::connect(&button, SIGNAL(clicked()), &callback, SLOT(invoke()));

  button.show();

  QTimer::singleShot(0, &callback, SLOT(invoke()));

  bool checkResult = false;
  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(500, &app, SLOT(quit()));
    checkResult = true;
    }

  int status = app.exec();
  if (status == EXIT_FAILURE)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with qSlicerApplication::exec()";
    return EXIT_FAILURE;
    }

  if (checkResult && !ErrorString.isEmpty())
    {
    std::cerr << "Line " << __LINE__ << " - Problem executing command line module - "
              << qPrintable(ErrorString) << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;

}
