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
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qMRML includes
#include "qMRMLClipNodeWidget.h"

// MRML includes
#include <vtkMRMLClipModelsNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qMRMLClipNodeWidget* widget = reinterpret_cast<qMRMLClipNodeWidget*>(data);

  CTKCOMPARE(widget->redSliceClipState(), vtkMRMLClipModelsNode::ClipPositiveSpace);
  CTKCOMPARE(widget->greenSliceClipState(), vtkMRMLClipModelsNode::ClipNegativeSpace);
  CTKCOMPARE(widget->clipType(), vtkMRMLClipModelsNode::ClipUnion);
  CTKCOMPARE(widget->yellowSliceClipState(), vtkMRMLClipModelsNode::ClipOff);
  }
}

//-----------------------------------------------------------------------------
int qMRMLClipNodeWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  qMRMLClipNodeWidget* widget = new qMRMLClipNodeWidget();

  vtkSmartPointer< vtkMRMLClipModelsNode > clipNode =
    vtkSmartPointer< vtkMRMLClipModelsNode >::New();

  widget->setMRMLClipNode(clipNode);

  etpWidget.addTestCase(widget,
                        xmlDirectory + "qMRMLClipNodeWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

