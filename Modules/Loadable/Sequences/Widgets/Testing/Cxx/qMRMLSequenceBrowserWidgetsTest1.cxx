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
#include <QApplication>
#include <QTimer>
#include <QVBoxLayout>

// qMRML includes
#include "qSlicerCoreApplication.h"
#include "qMRMLSequenceBrowserPlayWidget.h"
#include "qMRMLSequenceBrowserSeekWidget.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkNew.h>

int qMRMLSequenceBrowserWidgetsTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  const int numberOfDataNodes = 135;
  for (int i=0; i<numberOfDataNodes; i++)
    {
    vtkNew<vtkMRMLTransformNode> transform;
    QString indexValue = QString::number(i*1322.345);
    sequenceNode->SetDataNodeAtValue(transform.GetPointer(), indexValue.toLatin1().constData());
    }
  scene->AddNode(sequenceNode.GetPointer());

  vtkNew<vtkMRMLSequenceBrowserNode> browserNode;
  scene->AddNode(browserNode.GetPointer());
  browserNode->SetAndObserveMasterSequenceNodeID(sequenceNode->GetID());

  //
  // Create a simple gui with non-tranposed and transposed table view
  //
  QWidget parentWidget;
  parentWidget.setWindowTitle("qMRMLSequenceBrowserWidgetsTest1");
  QVBoxLayout vbox;
  parentWidget.setLayout(&vbox);

  qMRMLSequenceBrowserPlayWidget* playWidget = new qMRMLSequenceBrowserPlayWidget();
  playWidget->setParent(&parentWidget);
  playWidget->setMRMLSequenceBrowserNode(browserNode.GetPointer());
  vbox.addWidget(playWidget);

  qMRMLSequenceBrowserSeekWidget* seekWidget = new qMRMLSequenceBrowserSeekWidget();
  seekWidget->setParent(&parentWidget);
  seekWidget->setMRMLSequenceBrowserNode(browserNode.GetPointer());
  vbox.addWidget(seekWidget);

  parentWidget.show();
  parentWidget.raise();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}