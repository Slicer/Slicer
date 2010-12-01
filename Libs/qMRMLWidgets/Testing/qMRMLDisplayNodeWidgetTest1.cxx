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

// QT includes
#include <QApplication>
#include <QTimer>

// qMRML includes
#include "qMRMLDisplayNodeWidget.h"

// MRML includes
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLDisplayNodeWidgetTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);
  
  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode =
    vtkSmartPointer< vtkMRMLModelDisplayNode >::New();

  qMRMLDisplayNodeWidget displayNodeWidget;

  if (displayNodeWidget.isEnabled())
    {
    std::cerr << "No vtkMRMLDisplayNode provided, should be disabled."
              << std::endl;
    return EXIT_FAILURE;
    }

  double color[3];
  displayNode->GetColor(color);
  double opacity = displayNode->GetOpacity();
  double ambient = displayNode->GetAmbient();
  double diffuse = displayNode->GetDiffuse();
  double specular = displayNode->GetSpecular();
  double specularPower = displayNode->GetPower();
  
  bool backfaceCulling = displayNode->GetBackfaceCulling();
  
  displayNodeWidget.setMRMLDisplayNode(displayNode);
  
  if (displayNodeWidget.mrmlDisplayNode() != displayNode.GetPointer())
    {
    std::cerr << "qMRMLDisplayNodeWidget::setMRMLDisplayNode() failed."
              << displayNodeWidget.mrmlDisplayNode() << std::endl;
    return EXIT_FAILURE;
    }

  if (displayNodeWidget.color() != QColor::fromRgbF(color[0],color[1],color[2]))
    {
    std::cerr << "Wrong color: " << displayNodeWidget.color().rgb() << std::endl;
    return EXIT_FAILURE;
    }

  if (displayNodeWidget.opacity() != opacity)
    {
    std::cerr << "Wrong opacity: " << displayNodeWidget.opacity() << std::endl;
    return EXIT_FAILURE;
    }
  
  if (displayNodeWidget.ambient() != ambient)
    {
    std::cerr << "Wrong ambient: " << displayNodeWidget.ambient() << std::endl;
    return EXIT_FAILURE;
    }

  if (displayNodeWidget.diffuse() != diffuse)
    {
    std::cerr << "Wrong diffuse: " << displayNodeWidget.diffuse() << std::endl;
    return EXIT_FAILURE;
    }

  if (displayNodeWidget.specular() != specular)
    {
    std::cerr << "Wrong specular: " << displayNodeWidget.specular() << std::endl;
    return EXIT_FAILURE;
    }

  if (displayNodeWidget.specularPower() != specularPower)
    {
    std::cerr << "Wrong specularPower: " << displayNodeWidget.specularPower() << std::endl;
    return EXIT_FAILURE;
    }

  if (displayNodeWidget.backfaceCulling() != backfaceCulling)
    {
    std::cerr << "Wrong backfaceCulling: " << displayNodeWidget.backfaceCulling() << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->SetColor(1., 1., 1.);

  if (displayNodeWidget.color() != QColor::fromRgbF(1., 1., 1.))
    {
    std::cerr << "vtkMRMLDisplayNode::SetColor() failed: " << displayNodeWidget.color().rgb() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setColor(Qt::red);
  displayNode->GetColor(color);
  if (color[0] != 1. || color[1] != 0. || color[2] !=0)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setColor() failed: "
              << color[0] << " " << color[1] << " " << color[2] << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->SetOpacity(0.11);

  if (displayNodeWidget.opacity() != 0.11)
    {
    std::cerr << "vtkMRMLDisplayNode::SetOpacity() failed: " << displayNodeWidget.opacity() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setOpacity(0.99);
  
  if (displayNode->GetOpacity() != 0.99)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setOpacity() failed: "
              << displayNode->GetOpacity() << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->SetAmbient(0.5);

  if (displayNodeWidget.ambient() != 0.5)
    {
    std::cerr << "vtkMRMLDisplayNode::SetAmbient() failed: " << displayNodeWidget.ambient() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setAmbient(0.8);
  
  if (displayNode->GetAmbient() != 0.8)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setAmbient() failed: "
              << displayNode->GetAmbient() << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->SetDiffuse(1.2);

  if (displayNodeWidget.diffuse() != 1.)
    {
    std::cerr << "vtkMRMLDisplayNode::SetDiffuse() failed: " << displayNodeWidget.diffuse() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setDiffuse(0.3);
  
  if (displayNode->GetDiffuse() != 0.3)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setDiffuse() failed: "
              << displayNode->GetDiffuse() << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->SetSpecular(0.99);

  if (displayNodeWidget.specular() != 0.99)
    {
    std::cerr << "vtkMRMLDisplayNode::SetSpecular() failed: " << displayNodeWidget.specular() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setSpecular(0.01);
  
  if (displayNode->GetSpecular() != 0.01)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setSpecular() failed: "
              << displayNode->GetSpecular() << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->SetPower(45);

  if (displayNodeWidget.specularPower() != 45)
    {
    std::cerr << "vtkMRMLDisplayNode::SetPower() failed: " << displayNodeWidget.specularPower() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setSpecularPower(60);
  
  if (displayNode->GetPower() != 50)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setSpecularPower() failed: "
              << displayNode->GetPower() << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->SetBackfaceCulling(false);

  if (displayNodeWidget.backfaceCulling() != false)
    {
    std::cerr << "vtkMRMLDisplayNode::SetBackfaceCulling() failed: " << displayNodeWidget.backfaceCulling() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setBackfaceCulling(true);
  
  if (displayNode->GetBackfaceCulling() != 1)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setBackfaceCulling() failed: "
              << displayNode->GetBackfaceCulling() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.show();
  
  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

