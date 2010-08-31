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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerCoreIOManager.h"
#include "vtkMRMLScene.h"

#include <stdlib.h>
#include <iostream>
#include <QString>

#include "TestingMacros.h"

int qSlicerCoreIOManagerTest1(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Missing arguments" << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerCoreIOManager manager;

  vtkSmartPointer< vtkMRMLScene > scene1 = vtkSmartPointer< vtkMRMLScene >::New();

  QString filename = argv[1];

  //manager.loadScene( scene1, filename );

  vtkSmartPointer< vtkMRMLScene > scene2 = vtkSmartPointer< vtkMRMLScene >::New();

  //manager.loadScene( scene2, filename );

  //manager.closeScene( scene1 );

  QString extension = "mrml";

  //QString fileType = manager.fileTypeFromExtension( extension );

  std::cout << "File Type from extension " << qPrintable(extension);
  //std::cout << " is " << qPrintable(fileType) << std::endl;

  return EXIT_SUCCESS;
}
