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
#include "qMRMLThreeDWidget.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLThreeDWidgetTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);
  qMRMLThreeDWidget widget;
  widget.show();
  
  vtkNew<vtkMRMLViewNode> viewNode;

  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(viewNode.GetPointer());
  
  widget.setMRMLScene(scene.GetPointer());
  widget.setMRMLViewNode(viewNode.GetPointer());
  
  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

