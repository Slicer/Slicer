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

// Volumes includes
#include "qSlicerTractographyDisplayGlyphWidget.h"

// MRML includes
#include <vtkMRMLFiberBundleGlyphDisplayNode.h>
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
int qSlicerTractographyDisplayGlyphWidgetTest1( int argc, char * argv[] )
{
  QApplication app(argc, argv);

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLFiberBundleGlyphDisplayNode> fbDisplay;
  scene->AddNode(fbDisplay.GetPointer());
  vtkNew<vtkMRMLDiffusionTensorDisplayPropertiesNode> dtDisplayProperties;
  scene->AddNode(dtDisplayProperties.GetPointer());


  qSlicerTractographyDisplayGlyphWidget widget;
  widget.setFiberBundleDisplayNode(static_cast<vtkMRMLNode*>(0));

  widget.setFiberBundleDisplayNode(fbDisplay.GetPointer());

  fbDisplay->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(dtDisplayProperties->GetID());

  widget.setFiberBundleDisplayNode(fbDisplay.GetPointer());

  widget.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
