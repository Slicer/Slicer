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
#include <QWidget>

// SlicerQt includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes

// STD includes

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

int qMRMLLayoutManagerTest4(int argc, char * argv[] )
{
  QApplication app(argc, argv);

  QWidget w;
  w.show();

  qMRMLLayoutManager layoutManager(&w, &w);

  vtkMRMLScene* scene = vtkMRMLScene::New();

  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::New();
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->AddNode(layoutNode);
  layoutNode->Delete();

  layoutManager.setMRMLScene(scene);

  for (int i = vtkMRMLLayoutNode::SlicerLayoutInitialView;
       i < vtkMRMLLayoutNode::SlicerLayoutFourOverFourView; ++i)
    {
    layoutManager.setLayout(vtkMRMLLayoutNode::SlicerLayoutInitialView);
    scene->Clear(false);
    }

  scene->Delete();

  return EXIT_SUCCESS;
}

