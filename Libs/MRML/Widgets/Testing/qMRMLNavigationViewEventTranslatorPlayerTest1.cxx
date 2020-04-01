/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include "ctkCallback.h"
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qMRML includes
#include "qMRMLNavigationView.h"
#include "qMRMLThreeDView.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qMRMLNavigationView* widget = reinterpret_cast<qMRMLNavigationView*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qMRMLNavigationViewEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QString xmlDirectory = QString(argv[1]) + "/Libs/MRML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  etpWidget.setTestUtility(testUtility);

  // Test case 1
  QWidget topLevel;

  qMRMLNavigationView navigationView;
  navigationView.setWindowTitle("Navigation view");

  qMRMLThreeDView threeDView;
  threeDView.setWindowTitle("ThreeDView");

  QHBoxLayout* hboxLayout = new QHBoxLayout;
  hboxLayout->addWidget(&navigationView);
  hboxLayout->addWidget(&threeDView);
  topLevel.setLayout(hboxLayout);

  navigationView.setRendererToListen(threeDView.renderer());

  vtkNew<vtkMRMLScene> scene;

  // vtkMRMLAbstractDisplayableManager requires selection and interaction nodes
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());
  vtkNew<vtkMRMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  navigationView.setMRMLScene(scene.GetPointer());
  threeDView.setMRMLScene(scene.GetPointer());

  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::New();
  viewNode->SetBoxVisible(true);
  scene->AddNode(viewNode);
  viewNode->Delete();

  threeDView.setMRMLViewNode(viewNode);
  navigationView.setMRMLViewNode(viewNode);

  etpWidget.addTestCase(&topLevel,
                        xmlDirectory + "qMRMLNavigationViewEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

