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
#include <QApplication>
#include <QTimer>

// MRMLWidgets includes
#include <qMRMLTreeView.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes

int qMRMLModelTreeViewTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);
  
  vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
  vtkSmartPointer<vtkMRMLModelDisplayNode> displayModelNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  scene->AddNode(modelNode);
  scene->AddNode(displayModelNode);
  
  modelNode->SetAndObserveDisplayNodeID(displayModelNode->GetID());

  qMRMLTreeView modelView;
  modelView.setSceneModelType("ModelHierarchy");
  modelView.setMRMLScene(scene);

  modelView.show();
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, &app, SLOT(quit()));
    }

  return app.exec();
}
