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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Volumes logic
#include "vtkSlicerUnitsLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLUnitNode.h>

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
int vtkSlicerUnitsLogicTest1( int argc, char * argv[] )
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerUnitsLogic> logic;

  logic->SetMRMLScene(scene.GetPointer());

  std::vector<vtkMRMLNode*> nodes;
  scene->GetNodesByClass("vtkMRMLUnitNode", nodes);

  const int numberOfUnits = 2;
  const char* unitNodeIDs[numberOfUnits] = {"vtkMRMLUnitNodeApplicationLength",
                                            "vtkMRMLUnitNodeApplicationTime",
                                            };

  if (nodes.size() != numberOfUnits)
    {
    std::cerr<<"Not the right number of unit node in the scene !"<<std::endl;
    return EXIT_FAILURE;
    }

  for (int i = 0; i < numberOfUnits; ++i)
    {
    vtkMRMLUnitNode* node = vtkMRMLUnitNode::SafeDownCast(nodes[i]);
    if (!node || strcmp(node->GetID(), unitNodeIDs[i]) != 0)
      {
      std::cerr<<"Expecting node "<<unitNodeIDs[i]<<" Got: "<<
        (node ? node->GetID() : "")<<std::endl;
      return EXIT_FAILURE;
      }

    if (node->GetSaveWithScene())
      {
      std::cerr<<"Node "<<node->GetID()
        <<" should not be saved with the scene !"<<std::endl;
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}
