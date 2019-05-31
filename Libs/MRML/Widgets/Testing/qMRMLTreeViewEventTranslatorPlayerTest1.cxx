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
#include <QStandardItemModel>
#include <QSignalSpy>
#include <QTimer>
#include <QTreeView>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include "ctkCallback.h"
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qMRML includes
#include "qMRMLTreeView.h"
#include <qMRMLSceneTransformModel.h>
#include <qMRMLTreeViewEventPlayer.h>
#include <qMRMLTreeViewEventTranslator.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
QSignalSpy* SpyAction;
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qMRMLTreeView* widget = reinterpret_cast<qMRMLTreeView*>(data);

  CTKCOMPARE(widget->currentIndex().row(), 1);
  Q_UNUSED(widget);
  }
//-----------------------------------------------------------------------------
void checkFinalWidgetState2(void* data)
  {
  qMRMLTreeView* widget = reinterpret_cast<qMRMLTreeView*>(data);

  Q_UNUSED(widget);

  CTKCOMPARE(widget->currentIndex().row(), 1);
  }
}

//-----------------------------------------------------------------------------
int qMRMLTreeViewEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QString xmlDirectory = QString(argv[1]) + "/Libs/MRML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  testUtility->addPlayer(new qMRMLTreeViewEventPlayer());
  testUtility->addTranslator(new qMRMLTreeViewEventTranslator());
  etpWidget.setTestUtility(testUtility);

  // Test case 1
  qMRMLTreeView widget;

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer());
  widget.setMRMLScene(scene.GetPointer());
  scene->SetURL(argv[2]);
  scene->Import();

  etpWidget.addTestCase(&widget,
                        xmlDirectory + "qMRMLTreeViewEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // Test case 2
  qMRMLTreeView widget2;

  vtkNew<vtkMRMLModelNode> modelNode;
  vtkNew<vtkMRMLModelNode> modelNode2;
  vtkNew<vtkMRMLModelDisplayNode> displayModelNode;
  vtkNew<vtkMRMLModelDisplayNode> displayModelNode2;

  vtkNew<vtkMRMLScene> scene2;
  applicationLogic->SetMRMLScene(scene2.GetPointer());
  scene2->AddNode(modelNode.GetPointer());
  scene2->AddNode(modelNode2.GetPointer());
  scene2->AddNode(displayModelNode.GetPointer());
  scene2->AddNode(displayModelNode2.GetPointer());

  modelNode->SetAndObserveDisplayNodeID(displayModelNode->GetID());
  modelNode2->SetAndObserveDisplayNodeID(displayModelNode2->GetID());

  widget2.setSceneModelType("ModelHierarchy");
  widget2.setMRMLScene(scene2.GetPointer());

  QAction* insertTransformAction = new QAction("Insert transform", nullptr);
  widget2.prependNodeMenuAction(insertTransformAction);
  widget2.prependSceneMenuAction(insertTransformAction);

  QSignalSpy spyAction(insertTransformAction, SIGNAL(triggered()));
  SpyAction = &spyAction;

  etpWidget.addTestCase(&widget2,
                        xmlDirectory + "qMRMLTreeViewEventTranslatorPlayerTest2.xml",
                        &checkFinalWidgetState2);
  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();

  return app.exec();
}

