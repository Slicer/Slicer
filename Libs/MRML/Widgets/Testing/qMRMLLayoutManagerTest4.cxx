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

// Slicer includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

int qMRMLLayoutManagerTest4(int argc, char * argv[] )
{
  QApplication app(argc, argv);

  QWidget w;
  w.show();

  qMRMLLayoutManager layoutManager(&w, &w);

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLLayoutNode> layoutNode;

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->AddNode(layoutNode.GetPointer());

  applicationLogic->SetMRMLScene(scene.GetPointer());
  layoutManager.setMRMLScene(scene.GetPointer());

  for (int i = vtkMRMLLayoutNode::SlicerLayoutInitialView;
       i < vtkMRMLLayoutNode::SlicerLayoutFourOverFourView; ++i)
    {
    layoutManager.setLayout(vtkMRMLLayoutNode::SlicerLayoutInitialView);
    scene->Clear(false);
    }

  return EXIT_SUCCESS;
}
