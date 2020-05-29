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
#include <QDebug>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLNodeComboBox.h"

// MRML includes
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"


int qMRMLNodeComboBoxTest6( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkMRMLModelNode"));
  nodeSelector.setNoneEnabled(true);
  nodeSelector.addAttribute("vtkMRMLModelNode", "foo", 1);
/*
  qMRMLNodeComboBox nodeSelector2;
  nodeSelector2.setNodeTypes(QStringList("vtkMRMLModelNode"));
  nodeSelector2.setNoneEnabled(false);
  nodeSelector2.addAttribute("vtkMRMLModelNode", "foo", 0);
*/
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLModelNode> modelNode;
  modelNode->SetAttribute("foo", "1");
  scene->AddNode(modelNode.GetPointer());

  nodeSelector.setMRMLScene(scene.GetPointer());
  nodeSelector.setCurrentNode(modelNode.GetPointer());
  nodeSelector.show();
/*
  nodeSelector2.setMRMLScene(scene.GetPointer());
  nodeSelector2.setCurrentNode(modelNode.GetPointer());
  nodeSelector2.show();
*/
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  app.processEvents();
  modelNode->SetAttribute("foo", "0");
  modelNode->Modified();
  qDebug() <<"modified";
  return app.exec();
}
