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

// MRML includes
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <vtkNew.h>

//---------------------------------------------------------------------------
int vtkMRMLSceneAddSingletonTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  vtkNew<vtkMRMLScene> scene;

  // Add 3 singleton nodes
  // ID: vtkMRMLSliceNode1
  vtkNew<vtkMRMLSliceNode> redSlice;
  redSlice->SetLayoutName("Red");
  scene->AddNode(redSlice.GetPointer());

  // ID: vtkMRMLSliceNode2
  vtkNew<vtkMRMLSliceNode> yellowSlice;
  yellowSlice->SetLayoutName("Yellow");
  scene->AddNode(yellowSlice.GetPointer());

  // ID: vtkMRMLSliceNode3
  vtkNew<vtkMRMLSliceNode> greenSlice;
  greenSlice->SetLayoutName("Green");
  scene->AddNode(greenSlice.GetPointer());

  if (scene->GetNumberOfNodes() != 3 ||
      scene->GetNodeByID("vtkMRMLSliceNode1") != redSlice.GetPointer() ||
      scene->GetNodeByID("vtkMRMLSliceNode2") != yellowSlice.GetPointer() ||
      scene->GetNodeByID("vtkMRMLSliceNode3") != greenSlice.GetPointer())
    {
    std::cerr << "Invalid scene state: "
              << scene->GetNumberOfNodes() << " nodes." << std::endl
              << scene->GetNodeByID("vtkMRMLSliceNode1")
              << "/" << redSlice.GetPointer() << " "
              << scene->GetNodeByID("vtkMRMLSliceNode2")
              << "/" << yellowSlice.GetPointer() << " "
              << scene->GetNodeByID("vtkMRMLSliceNode3")
              << "/" << greenSlice.GetPointer() << std::endl;
    return EXIT_FAILURE;
    }

  // Add a singleton with the same tag but a different ID
  vtkNew<vtkMRMLSliceNode> otherRedSlice;
  otherRedSlice->SetLayoutName("Red");
  otherRedSlice->UpdateID("vtkMRMLSliceNode3");
  // AddNode(otherRedSlice) erases the green singleton from the Node ID cache,
  // the green node is still in the scene though.
  // For now, until the otherGreenSlice is added, the greenslice node can't be
  // found with GetNodeById("vtkMRMLSliceNode3") because otherRedSlice is
  // returned.
  scene->AddNode(otherRedSlice.GetPointer());

  // Add a singleton with the same tag but a different ID
  vtkNew<vtkMRMLSliceNode> otherGreenSlice;
  otherGreenSlice->SetLayoutName("Green");
  otherGreenSlice->UpdateID("vtkMRMLSliceNode1");
  scene->AddNode(otherGreenSlice.GetPointer());
  
  if (scene->GetNumberOfNodes() != 3 ||
      scene->GetNodeByID("vtkMRMLSliceNode1") != greenSlice.GetPointer() ||
      scene->GetNodeByID("vtkMRMLSliceNode2") != yellowSlice.GetPointer() ||
      scene->GetNodeByID("vtkMRMLSliceNode3") != redSlice.GetPointer())
    {
    std::cerr << "Invalid scene state: "
              << scene->GetNumberOfNodes() << " nodes." << std::endl
              << scene->GetNodeByID("vtkMRMLSliceNode1")
              << "/" << greenSlice.GetPointer() << " "
              << scene->GetNodeByID("vtkMRMLSliceNode2")
              << "/" << yellowSlice.GetPointer() << " "
              << scene->GetNodeByID("vtkMRMLSliceNode3")
              << "/" << redSlice.GetPointer() << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
