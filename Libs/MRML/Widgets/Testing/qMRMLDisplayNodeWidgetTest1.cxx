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

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLDisplayNodeWidget.h"

// MRML includes
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLDisplayNodeWidgetTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode =
    vtkSmartPointer< vtkMRMLModelDisplayNode >::New();

  qMRMLDisplayNodeWidget displayNodeWidget;

  if (displayNodeWidget.isEnabled())
    {
    std::cerr << "No vtkMRMLDisplayNode provided, should be disabled."
              << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setMRMLDisplayNode(displayNode);

  if (displayNodeWidget.mrmlDisplayNode() != displayNode.GetPointer())
    {
    std::cerr << "qMRMLDisplayNodeWidget::setMRMLDisplayNode() failed."
              << displayNodeWidget.mrmlDisplayNode() << std::endl;
    return EXIT_FAILURE;
    }

  bool visibility = displayNode->GetVisibility();
  if (displayNodeWidget.visibility() != visibility)
    {
    std::cerr << "Wrong visibility: " << displayNodeWidget.visibility() << std::endl;
    return EXIT_FAILURE;
    }

  bool selected = displayNode->GetSelected();
  if (displayNodeWidget.selected() != selected)
    {
    std::cerr << "Wrong selected: " << displayNodeWidget.selected() << std::endl;
    return EXIT_FAILURE;
    }

  bool clipping = displayNode->GetClipping();
  if (displayNodeWidget.clipping() != clipping)
    {
    std::cerr << "Wrong clipping: " << displayNodeWidget.clipping() << std::endl;
    return EXIT_FAILURE;
    }

  bool sliceIntersectionVisible = displayNode->GetVisibility2D();
  if (displayNodeWidget.sliceIntersectionVisible() != sliceIntersectionVisible)
    {
    std::cerr << "Wrong intersection: "
              << displayNodeWidget.sliceIntersectionVisible() << std::endl;
    return EXIT_FAILURE;
    }

  int sliceIntersectionThickness = displayNode->GetSliceIntersectionThickness();
  if (displayNodeWidget.sliceIntersectionThickness() != sliceIntersectionThickness)
    {
    std::cerr << "Wrong intersection thickness: "
              << displayNodeWidget.sliceIntersectionThickness() << std::endl;
    return EXIT_FAILURE;
    }

  displayNode->VisibilityOff();
  if (displayNodeWidget.visibility() != false)
    {
    std::cerr << "vtkMRMLDisplayNode::SetVisibility() failed: "
              << displayNodeWidget.visibility() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.setVisibility(true);
  if (displayNode->GetVisibility() != 1)
    {
    std::cerr << "qMRMLDisplayNodeWidget::setVisibility failed: "
              << displayNode->GetVisibility() << std::endl;
    return EXIT_FAILURE;
    }

  displayNodeWidget.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

