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

// QT includes
#include <QApplication>
#include <QTimer>

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
#include <vtkNew.h>

int qMRMLColorPickerWidgetTest2(int argc, char * argv [])
{
  QApplication app(argc, argv);

  qMRMLColorPickerWidget colorPickerWidget;

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLColorTableNode> colorTableNode;
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);
  scene->AddNode(colorTableNode.GetPointer());

  vtkNew<vtkMRMLFreeSurferProceduralColorNode> colorFreeSurferNode;
  colorFreeSurferNode->SetTypeToRedBlue();
  scene->AddNode(colorFreeSurferNode.GetPointer());

  colorPickerWidget.setMRMLScene(scene.GetPointer());

  // for some reasons it generate a warning if the type is changed.
  colorTableNode->NamesInitialisedOff();
  colorTableNode->SetTypeToCool1();

  vtkNew<vtkMRMLPETProceduralColorNode> colorPETNode;
  colorPETNode->SetTypeToRainbow();
  scene->AddNode(colorPETNode.GetPointer());

  ctkColorDialog::addDefaultTab(&colorPickerWidget, "Extra", SIGNAL(colorSelected(QColor)));

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    // quits the getColor dialog event loop.
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  ctkColorDialog::getColor(Qt::red, 0, "", 0);
  return EXIT_SUCCESS;
}

