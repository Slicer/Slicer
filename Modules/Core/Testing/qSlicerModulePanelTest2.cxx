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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDockWidget>
#include <QTimer>

// SlicerQt includes
#include <qSlicerApplication.h>
#include <qSlicerCoreModuleFactory.h>
#include <qSlicerModuleManager.h>
#include <qSlicerModuleFactoryManager.h>
#include <qSlicerModulePanel.h>
#include <qSlicerStyle.h>

// STD includes

int qSlicerModulePanelTest2(int argc, char * argv[] )
{
  QApplication::setDesktopSettingsAware(false);
  QApplication::setStyle(new qSlicerStyle);
  qSlicerApplication app(argc, argv);

  // Register core module factories for testing purpose
  qSlicerCoreModuleFactory* coreModuleFactory = new qSlicerCoreModuleFactory();
  app.moduleManager()->factoryManager()->registerFactory(coreModuleFactory);
  app.moduleManager()->factoryManager()->registerModules();
  app.moduleManager()->factoryManager()->instantiateModules();
  app.moduleManager()->factoryManager()->loadModules();

  QDockWidget dockWidget;
  qSlicerModulePanel* modulePanel = new qSlicerModulePanel(&dockWidget);
  dockWidget.setWidget(modulePanel);
  //QHBoxLayout* hbox = new QHBoxLayout;
  //hbox->addWidget(&modulePanel);
  //dockWidget.setLayout(hbox);

  modulePanel->setModuleManager(app.moduleManager());

  modulePanel->setModule("Cameras");
  if (modulePanel->currentModuleName() != "Cameras")
    {
    std::cerr<< "qSlicerModulePanel::setModule() failed "
             << qPrintable(modulePanel->currentModuleName()) << std::endl;
    return EXIT_FAILURE;
    }
  modulePanel->setModule("EventBroker");

  dockWidget.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, qApp, SLOT(quit()));
    }

  return app.exec();
}

