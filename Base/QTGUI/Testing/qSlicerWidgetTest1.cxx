/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QWidget>

// SlicerQt includes
#include "qSlicerWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

// STD includes
#include <cstdlib>

int qSlicerWidgetTest1(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget parentWidget;
  qSlicerWidget* widget = new qSlicerWidget();
  if (widget->mrmlScene() != 0)
    {
    std::cerr << "scene incorrectly initialized." << std::endl;
    return EXIT_FAILURE;
    }
  if (widget->parentWidget() != 0)
    {
    std::cerr << "parent widget incorrectly initialized." << std::endl;
    return EXIT_FAILURE;
    }
  widget->setParent(&parentWidget);
  if (widget->parentWidget() != &parentWidget)
    {
    std::cerr << "parent widget incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  if (widget->isParentContainerScrollArea())
    {
    std::cerr << "parent widget is not a scroll area." << std::endl;
    return EXIT_FAILURE;
    }
  QRect parentGeometry(10,10,20,20);
  widget->setParentGeometry( parentGeometry.x(), parentGeometry.y(), 
    parentGeometry.width(), parentGeometry.height());
  if (parentWidget.geometry() != parentGeometry)
    {
    std::cerr << "parent geometry incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  // check for infinite loop
  QObject::connect(widget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), 
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));
  vtkMRMLScene* scene = vtkMRMLScene::New();
  widget->setMRMLScene(scene);
  if (widget->mrmlScene() != scene)
    {
    std::cerr << "scene incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  scene->Delete();
  if (widget->mrmlScene() != scene)
    {
    std::cerr << "scene has been deleted, qSlicerWidget is supposed to keep a ref on it." << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

