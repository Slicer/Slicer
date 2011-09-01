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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QApplication>

// CTK includes
#include <ctkColorDialog.h>

// qMRML includes
#include "qMRMLColorPickerWidget.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLFreeSurferProceduralColorNode.h>
#include <vtkMRMLPETProceduralColorNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes

int qMRMLColorPickerWidgetTest2(int argc, char * argv [])
{
  QApplication app(argc, argv);

  qMRMLColorPickerWidget colorPickerWidget;

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkSmartPointer<vtkMRMLColorTableNode> colorTableNode =
    vtkSmartPointer<vtkMRMLColorTableNode>::New();
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);
  scene->AddNode(colorTableNode);
  
  vtkSmartPointer<vtkMRMLFreeSurferProceduralColorNode> colorFreeSurferNode =
    vtkSmartPointer<vtkMRMLFreeSurferProceduralColorNode>::New();
  colorFreeSurferNode->SetTypeToRedBlue();
  scene->AddNode(colorFreeSurferNode);

  colorPickerWidget.setMRMLScene(scene);

  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkSmartPointer<vtkMRMLPETProceduralColorNode> colorPETNode =
    vtkSmartPointer<vtkMRMLPETProceduralColorNode>::New();
  colorPETNode->SetTypeToRainbow();
  scene->AddNode(colorPETNode);
  
  ctkColorDialog::addDefaultTab(&colorPickerWidget, "Extra", SIGNAL(colorSelected(QColor)));

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    return EXIT_SUCCESS;
    }
  QColor color = ctkColorDialog::getColor(Qt::red, 0, "", 0);
  return EXIT_SUCCESS;
}

