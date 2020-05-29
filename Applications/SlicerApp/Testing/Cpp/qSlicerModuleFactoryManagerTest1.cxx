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

// SlicerApp includes
#include <qSlicerModuleFactoryManager.h>
#include <qSlicerCoreModuleFactory.h>
#include <qSlicerCoreApplication.h>

// STD includes


int qSlicerModuleFactoryManagerTest1(int argc, char * argv[])
{
  qSlicerCoreApplication app(argc, argv);
  Q_UNUSED(app);

  qSlicerModuleFactoryManager moduleFactoryManager;

  // Register factories
  moduleFactoryManager.registerFactory(new qSlicerCoreModuleFactory());

  // Register core modules
  moduleFactoryManager.registerModules();

  QString moduleName = "EventBroker";

  moduleFactoryManager.instantiateModules();
  moduleFactoryManager.loadModules();

  qSlicerAbstractCoreModule * abstractModule =
    moduleFactoryManager.moduleInstance(moduleName);
  if( abstractModule == nullptr )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in loadModule()" << std::endl;
    return EXIT_FAILURE;
    }

  if( abstractModule->name() != moduleName )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in moduleTitle() or moduleName()" << std::endl
              << "expected moduleName  = " << qPrintable( moduleName ) << std::endl
              << "real moduleName = " << qPrintable( abstractModule->name() ) << std::endl;
    return EXIT_FAILURE;
    }

  moduleFactoryManager.unloadModules();

  // Instantiate again
  moduleFactoryManager.instantiateModules();
  moduleFactoryManager.loadModules();
  abstractModule = moduleFactoryManager.moduleInstance(moduleName);

  if( abstractModule == nullptr )
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in instantiateModule()" << std::endl;
    return EXIT_FAILURE;
    }

  // Check failure cases (if loading of modules does not work then other tests will fail,
  // but module loading failures are not tested elsewhere)

  // This should not crash
  moduleFactoryManager.registerModule(QFileInfo("nonexistent"));

  std::cout << "The following module loading is expected to fail..." << std::endl;
  bool moduleLoadSuccess = moduleFactoryManager.loadModules(QStringList() << "nonexistent2");
  if (moduleLoadSuccess == true)
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in loadModules() - it expected to fail for non-existent module" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Module loading failed as expected." << std::endl;

  moduleLoadSuccess = moduleFactoryManager.loadModules(QStringList());
  if (moduleLoadSuccess == false)
    {
    moduleFactoryManager.printAdditionalInfo();
    std::cerr << __LINE__ << " - Error in loadModules() - it expected to succeed for empty module list" << std::endl;
    return EXIT_FAILURE;
    }

  moduleFactoryManager.unloadModules();

  return EXIT_SUCCESS;
}

