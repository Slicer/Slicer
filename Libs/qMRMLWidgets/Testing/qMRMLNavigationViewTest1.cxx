/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// QT includes
#include <QApplication>
#include <QTimer>

// qMRML includes
#include "qMRMLNavigationView.h"
#include "qMRMLThreeDView.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLNavigationViewTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);
  
  qMRMLNavigationView navigationView;
  navigationView.setWindowTitle("Navigation view");

  qMRMLThreeDView threeDView;
  threeDView.setWindowTitle("ThreeDView");
  navigationView.setRendererToListen(threeDView.renderer());
  
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  navigationView.setMRMLScene(scene);
  threeDView.setMRMLScene(scene);
  
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::New();
  viewNode->SetBoxVisible(true);
  scene->AddNode(viewNode);
  viewNode->Delete();
  
  threeDView.setMRMLViewNode(viewNode);
  navigationView.setMRMLViewNode(viewNode);

  navigationView.show();
  threeDView.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  
  return app.exec();
}
