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
#include <QTimer>
#include <QTreeView>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include "ctkCallback.h"
#include "ctkColorDialog.h"
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qMRML includes
#include "qMRMLColorPickerWidget.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLPETProceduralColorNode.h>
#include <vtkMRMLScene.h>

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

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
  qMRMLColorPickerWidget* widget = reinterpret_cast<qMRMLColorPickerWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qMRMLColorPickerWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  qMRMLColorPickerWidget* widget = new qMRMLColorPickerWidget();
  widget->setObjectName("ColorPickerWidget1");

  vtkNew<vtkMRMLScene> scene;

  widget->setMRMLScene(scene.GetPointer());
  vtkNew<vtkMRMLColorLogic> colorLogic;
  colorLogic->SetMRMLScene(scene.GetPointer());

  etpWidget.addTestCase(widget,
                        xmlDirectory + "qMRMLColorPickerWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // Test case 2
  qMRMLColorPickerWidget* widget2 = new qMRMLColorPickerWidget();
  widget2->setObjectName("ColorPickerWidget2");

  vtkNew<vtkMRMLScene> scene2;

  vtkNew<vtkMRMLColorTableNode> colorTableNode;
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);
  scene2->AddNode(colorTableNode.GetPointer());

  widget2->setMRMLScene(scene2.GetPointer());

  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkNew<vtkMRMLPETProceduralColorNode> colorPETNode;
  colorPETNode->SetTypeToRainbow();
  scene2->AddNode(colorPETNode.GetPointer());

  etpWidget.addTestCase(widget2,
                        xmlDirectory + "qMRMLColorPickerWidgetEventTranslatorPlayerTest2.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

