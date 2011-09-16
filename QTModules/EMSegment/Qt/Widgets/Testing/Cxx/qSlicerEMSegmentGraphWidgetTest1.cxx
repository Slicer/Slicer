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

// EMSegment includes
#include "vtkEMSegmentLogic.h"
#include "qSlicerEMSegmentGraphWidget.h"

// MRML includes
#include "vtkMRMLScene.h"

// STD includes
#include <iostream>

int qSlicerEMSegmentGraphWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);
  qSlicerEMSegmentGraphWidget graph;
  graph.show();

  if (argc < 2)
    {
    return EXIT_SUCCESS;
    }

  vtkEMSegmentLogic* logic = vtkEMSegmentLogic::New();
  vtkMRMLScene* scene =vtkMRMLScene::New();
  logic->SetMRMLScene(scene);
  scene->SetURL(argv[1]);
  scene->Import();

  graph.setMRMLManager(logic->GetMRMLManager());
  return EXIT_SUCCESS;
}
