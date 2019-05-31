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
#include "qMRMLVolumeInfoWidget.h"

// MRML includes
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkVersion.h>
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qMRMLVolumeInfoWidget* widget = reinterpret_cast<qMRMLVolumeInfoWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qMRMLVolumeInfoWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
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
  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;

  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(256, 256, 1);
  //imageData->SetSpacing(2., 2., 512.); not used by vtkMRMLVolumeNode
  //imageData->SetOrigin(0.0,0.0,0.0); not used by vtkMRMLVolumeNode
  imageData->AllocateScalars(VTK_UNSIGNED_SHORT, 1); // allocate storage for image data
  volumeNode->SetAndObserveImageData(imageData.GetPointer());
  volumeNode->SetSpacing(2., 2., 512.);
  volumeNode->SetOrigin(0, 0, 0);

  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  vtkNew<vtkMRMLScene> scene;

  // Add default color nodes
  vtkNew<vtkMRMLColorLogic> colorLogic;
  colorLogic->SetMRMLScene(scene.GetPointer());

  scene->AddNode(volumeNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());

  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  qMRMLVolumeInfoWidget volumeInfo;
  volumeInfo.setVolumeNode(volumeNode.GetPointer());
  etpWidget.addTestCase(&volumeInfo,
                        xmlDirectory + "qMRMLVolumeInfoWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

