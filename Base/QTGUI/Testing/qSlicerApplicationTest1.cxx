/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerCommandOptions.h"
#include "qSlicerIOManager.h"

// STD includes
#include <cstdlib>

int qSlicerApplicationTest1(int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);

  if (app.commandOptions() == 0)
    {
    std::cerr << "Problem with commandOptions()" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCommandOptions * commandOptions = new qSlicerCommandOptions;
  app.setCoreCommandOptions(commandOptions);

  qSlicerCommandOptions * commandOptions2 = app.commandOptions();
  if (commandOptions2 != commandOptions)
    {
    std::cerr << "Problem with setCommandOptions()/commandOptions()" << std::endl;
    return EXIT_FAILURE;
    }

  if (app.ioManager() == 0)
    {
    std::cerr << "Problem with ioManager()" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerIOManager * ioManager = new qSlicerIOManager;
  app.setCoreIOManager(ioManager);

  qSlicerIOManager * ioManager2 = app.ioManager();
  if(ioManager2 != ioManager)
    {
    std::cerr << "Problem with setIOManager()/ioManager()" << std::endl;
    return EXIT_FAILURE;
    }

  bool exitWhenDone = false;
  app.parseArguments(exitWhenDone);
  if (exitWhenDone)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with parseArguments()" << std::endl;
    return EXIT_FAILURE;
    }

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, qApp, SLOT(quit()));
    }

  return app.exec();
}

