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
#include "qMRMLScalarInvariantComboBox.h"

// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLScalarInvariantComboBoxTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLDiffusionTensorDisplayPropertiesNode> displayPropertiesNode;

  qMRMLScalarInvariantComboBox scalarComboBox;
  scalarComboBox.setDisplayPropertiesNode(displayPropertiesNode.GetPointer());

  displayPropertiesNode->SetColorGlyphBy(
    vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity);
  if (scalarComboBox.scalarInvariant() != vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity)
    {
    std::cerr << "qMRMLScalarInvariantComboBox failed: "
              << scalarComboBox.scalarInvariant() << " instead of "
              << displayPropertiesNode->GetColorGlyphBy() << std::endl;
    return EXIT_FAILURE;
    }

  scalarComboBox.setScalarInvariant(
    vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
  if (displayPropertiesNode->GetColorGlyphBy() !=
      vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy)
    {
    std::cerr << "qMRMLScalarInvariantComboBox::setScalarInvariant() failed: "
              << displayPropertiesNode->GetColorGlyphBy() << " instead of "
              << scalarComboBox.scalarInvariant() << std::endl;
    return EXIT_FAILURE;
    }

  scalarComboBox.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

