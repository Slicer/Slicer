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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QString>
#include <QStringList>

// SlicerQt includes
#include <qSlicerModuleFactoryManager.h>
#include <qSlicerCoreModuleFactory.h>
#include <qSlicerCoreApplication.h>

// STD includes
#include <cstdlib>
#include <iostream>

#include "TestingMacros.h"

int qSlicerModuleFactoryManagerTest1(int argc, char * argv[])
{
  qSlicerCoreApplication app(argc, argv);
  Q_UNUSED(app);
  
  qSlicerModuleFactoryManager moduleFactoryManager;

  // Register factories
  moduleFactoryManager.registerFactory("qSlicerCoreModuleFactory", new qSlicerCoreModuleFactory());

  // Register core modules
  moduleFactoryManager.registerModules("qSlicerCoreModuleFactory");

  QString moduleName = "transforms";
  
  if (!moduleFactoryManager.moduleTitle( moduleName ).isEmpty())
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error: Registered module " << qPrintable(moduleName)
                          << " can't have a valid title yet."
                          << " Titles get set after the module get instantiated." << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerAbstractCoreModule * abstractModule = moduleFactoryManager.instantiateModule( moduleName );

  if( abstractModule == NULL )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in instantiateModule()" << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle = moduleFactoryManager.moduleTitle( moduleName );
  QString moduleName1 = moduleFactoryManager.moduleName( moduleTitle );
  
  if( moduleName1 != moduleName )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in moduleTitle() or moduleName()" << std::endl
                          << "moduleTitle  = " << qPrintable( moduleTitle ) << std::endl
                          << "moduleName  = " << qPrintable( moduleName ) << std::endl
                          << "moduleName1 = " << qPrintable( moduleName1 ) << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle1 = moduleFactoryManager.moduleTitle( moduleName );

  if( moduleTitle1 != moduleTitle )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in getModuleTitle()" << std::endl
                          << "moduleTitle  = " << qPrintable( moduleTitle ) << std::endl
                          << "moduleTitle1 = " << qPrintable( moduleTitle1 ) << std::endl;
    return EXIT_FAILURE;
    }

  QString moduleTitle2 = abstractModule->title(); 
  
  if( moduleTitle2 != moduleTitle )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in title():" << qPrintable(moduleTitle2) << std::endl
                          << "moduleTitle  = " << qPrintable( moduleTitle ) << std::endl
                          << "moduleTitle2 = " << qPrintable( moduleTitle2 ) << std::endl;
    return EXIT_FAILURE;
    }

  moduleFactoryManager.uninstantiateModule( moduleName );

  // Instantiate again
  abstractModule = moduleFactoryManager.instantiateModule( moduleName );

  if( abstractModule == NULL )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in instantiateModule()" << std::endl;
    return EXIT_FAILURE;
    }

  moduleFactoryManager.uninstantiateAll();

  return EXIT_SUCCESS;
}

