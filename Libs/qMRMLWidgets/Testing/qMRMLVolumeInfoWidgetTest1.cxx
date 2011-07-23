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
#include <QTimer>

// qMRML includes
#include "qMRMLVolumeInfoWidget.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLVolumeInfoWidgetTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);
  
  vtkSmartPointer< vtkMRMLScalarVolumeNode > volumeNode = vtkSmartPointer< vtkMRMLScalarVolumeNode >::New();

  vtkSmartPointer< vtkImageData > imageData = vtkSmartPointer< vtkImageData >::New();
  imageData->SetDimensions(256, 256, 1);
  imageData->SetScalarTypeToUnsignedShort();
  imageData->SetNumberOfScalarComponents(1); // image holds one value intensities
  //imageData->SetSpacing(2., 2., 512.); not used by vtkMRMLVolumeNode
  //imageData->SetOrigin(0.0,0.0,0.0); not used by vtkMRMLVolumeNode
  imageData->AllocateScalars(); // allocate storage for image data  
  
  volumeNode->SetAndObserveImageData(imageData);
  volumeNode->SetSpacing(2., 2., 512.);
  volumeNode->SetOrigin(0, 0, 0);

  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> displayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  scene->AddNode(volumeNode);
  scene->AddNode(displayNode);

  vtkSmartPointer<vtkMRMLColorTableNode> colorNode = vtkSmartPointer<vtkMRMLColorTableNode>::New();
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode);
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  
  qMRMLVolumeInfoWidget volumeInfo;
  volumeInfo.setVolumeNode(volumeNode);
  volumeInfo.show();
  
  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

