/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// SlicerQT includes
#include "qSlicerCoreApplication.h"
#include "qSlicerTransformsModule.h"
#include "qSlicerCoreCommandOptions.h"

// STD includes
#include <stdlib.h>

#include "TestingMacros.h"

int qSlicerTransformsModuleTest1(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Missing arguments" << std::endl;
    return EXIT_FAILURE;
    }

  int argc2 = argc - 1;
  char ** argv2 = argv + 1;

  qSlicerCoreApplication app( argc2, argv2 );

  qSlicerCoreApplication * aptr = app.application();

  if( aptr != (&app) )
    {
    std::cerr << "Problem with the application() singleton" << std::endl;
    return EXIT_FAILURE;
    }

  QSettings * settings = app.settings();
  if( settings == 0 )
    {
    std::cerr << "Problem with settings()" << std::endl;
    return EXIT_FAILURE;
    }

  if( app.coreCommandOptions() == 0 )
    {
    std::cerr << "Problem with coreCommandOptions()" << std::endl;
    return EXIT_FAILURE;
    }

  bool exitWhenDone = false;
  aptr->parseArguments(exitWhenDone);
  if (exitWhenDone == true)
    {
    std::cerr << "Problem with the application::parseArguments function" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerTransformsModule * transformsModule = new qSlicerTransformsModule;
  transformsModule->initialize(aptr->appLogic());

  if (transformsModule->logic() != transformsModule->logic())
    {
    std::cerr << "The logic must be always the same." << std::endl;
    return EXIT_FAILURE;
    }

  if (transformsModule->widgetRepresentation() != transformsModule->widgetRepresentation())
    {
    std::cerr << "The logic must be always the same." << std::endl;
    return EXIT_FAILURE;
    }
  delete transformsModule;

  return EXIT_SUCCESS;
}

