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
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <sstream>

namespace
{

void PopulateScene(vtkMRMLScene* scene, int numberOfLevels);
vtkMRMLModelNode* GetModelNode(vtkMRMLScene* scene, int level);
vtkMRMLDisplayableHierarchyNode* GetHierarchyNode(vtkMRMLScene* scene, int level);
vtkMRMLDisplayableHierarchyNode* GetModelHierarchyNode(vtkMRMLScene* scene, int level);

bool TestExpand();
bool TestDefaults();
bool TestSetVisibilityLevel0();
bool TestSetVisibilityLevel1();
bool TestCollapseLevel0();
bool TestCollapseLevel1();
bool TestCollapseLevels0And1();

bool TestColors(vtkMRMLScene* scene,
                double (*modelColors)[3],
                double (*hierarchyColors)[3]);
bool TestVisibility(vtkMRMLScene* scene,
                    int* modelVisibilities,
                    int* hierarchyVisibilities);

const int LEVEL_COUNT = 3;
double DEFAULT_MODEL_COLORS[3][3] = {{0.5, 0., 0.}, {0.,0.5,0.},{0.,0.,0.5}};
double DEFAULT_HIERARCHY_COLORS[3][3] = {{1., 0., 0.}, {0.,1.,0.},{0.,0.,1.}};
int DEFAULT_MODEL_VISIBILITY[3] = {1, 1, 1};
int DEFAULT_HIERARCHY_VISIBILITY[3] = {1, 1, 1};

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLDisplayableHierarchyNodeDisplayPropertiesTest(int vtkNotUsed(argc),
                                                         char * vtkNotUsed(argv)[] )
{
  if (!TestExpand())
    {
    std::cerr << "'TestExpand' call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
// Populate a scene with models and hierarchy
// Scene
//   +- ModelHierarchyNode (level 0)
//   |     > ModelNode
//   |          > DisplayNode
//   + HierarchyNode (level 0)
//      | > DisplayNode
//      + ModelHierarchyNode (level 1)
//      |    > ModelNode
//      |         > DisplayNode
//      + HierarchyNode (level 1)
//         | > DisplayNode
//         + ModelHierarchyNode (level 2)
//         |    > ModelNode
//         |         > DisplayNode
//         + HierarchyNode (level 2)
//              > DisplayNode
void PopulateScene(vtkMRMLScene* scene, int numberOfLevels)
{
  // At each level, create a Model node and a Hierarchy node, the latter becomes
  // the parent of the next level.
  vtkMRMLDisplayableHierarchyNode* parentHierarchy = nullptr;
  for (int level = 0; level < numberOfLevels; ++level)
    {
    // Model
    vtkNew<vtkMRMLModelNode> modelNode;
    std::stringstream smn;
    smn << level << " model";
    modelNode->SetName(smn.str().c_str());
    scene->AddNode(modelNode.GetPointer());

    vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
    std::stringstream smdn;
    smdn << level << " model display";
    modelDisplayNode->SetName(smdn.str().c_str());
    scene->AddNode(modelDisplayNode.GetPointer());

    vtkNew<vtkMRMLModelHierarchyNode> modelHierarchyNode;
    std::stringstream smhn;
    smhn << level << " model hierarchy";
    modelHierarchyNode->SetName(smhn.str().c_str());
    scene->AddNode(modelHierarchyNode.GetPointer());

    modelNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());
    modelHierarchyNode->SetDisplayableNodeID(modelNode->GetID());
    modelDisplayNode->SetColor(DEFAULT_MODEL_COLORS[level]);

    // Hierarchy
    vtkNew<vtkMRMLDisplayableHierarchyNode> hierarchyNode;
    std::stringstream shn;
    shn << level << " hierarchy";
    hierarchyNode->SetName(shn.str().c_str());
    scene->AddNode(hierarchyNode.GetPointer());

    vtkNew<vtkMRMLModelDisplayNode> hierarchyDisplayNode;
    std::stringstream shdn;
    shdn << level << " hierarchy display";
    hierarchyDisplayNode->SetName(shdn.str().c_str());
    scene->AddNode(hierarchyDisplayNode.GetPointer());

    hierarchyNode->SetAndObserveDisplayNodeID(hierarchyDisplayNode->GetID());
    hierarchyDisplayNode->SetColor(DEFAULT_HIERARCHY_COLORS[level]);
    if (parentHierarchy)
      {
      modelHierarchyNode->SetParentNodeID(parentHierarchy->GetID());
      hierarchyNode->SetParentNodeID(parentHierarchy->GetID());
      }
    parentHierarchy = hierarchyNode.GetPointer();
    }
  assert(scene->GetNumberOfNodes() == numberOfLevels*5);
}

//---------------------------------------------------------------------------
vtkMRMLModelNode* GetModelNode(vtkMRMLScene* scene, int level)
{
  return vtkMRMLModelNode::SafeDownCast(
    scene->GetNthNodeByClass(level, "vtkMRMLModelNode"));
}

//---------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode* GetHierarchyNode(vtkMRMLScene* scene, int level)
{
  return vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    scene->GetNthNodeByClass(2*level + 1, "vtkMRMLDisplayableHierarchyNode"));
}

//---------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode* GetModelHierarchyNode(vtkMRMLScene* scene, int level)
{
  return vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    scene->GetNthNodeByClass(2*level, "vtkMRMLDisplayableHierarchyNode"));
}

//---------------------------------------------------------------------------
bool TestExpand()
{
  if (!TestDefaults())
    {
    std::cerr << "'TestDefaults' call not successful." << std::endl;
    return false;
    }
  if (!TestSetVisibilityLevel0())
    {
    std::cerr << "'TestSetVisibilityLevel0' call not successful." << std::endl;
    return false;
    }
  if (!TestSetVisibilityLevel1())
    {
    std::cerr << "'TestSetVisibilityLevel0' call not successful." << std::endl;
    return false;
    }
  if (!TestCollapseLevel0())
    {
    std::cerr << "'TestCollapseLevel0' call not successful." << std::endl;
    return false;
    }
  if (!TestCollapseLevel1())
    {
    std::cerr << "'TestCollapseLevel1' call not successful." << std::endl;
    return false;
    }
  if (!TestCollapseLevels0And1())
    {
    std::cerr << "'TestCollapseLevels0And1' call not successful." << std::endl;
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool TestDefaults()
{
  vtkNew<vtkMRMLScene> scene;
  PopulateScene(scene.GetPointer(), LEVEL_COUNT);
  bool res = true;
  res = TestColors(scene.GetPointer(), DEFAULT_MODEL_COLORS, DEFAULT_HIERARCHY_COLORS) && res;
  res = TestVisibility(scene.GetPointer(), DEFAULT_MODEL_VISIBILITY, DEFAULT_HIERARCHY_VISIBILITY) && res;
  return res;
}

//---------------------------------------------------------------------------
bool TestSetVisibilityLevel0()
{
  vtkNew<vtkMRMLScene> scene;
  PopulateScene(scene.GetPointer(), LEVEL_COUNT);

  // Color and visibility shouldn't be changed when collapsing hierarchy
  vtkMRMLDisplayableHierarchyNode* hierarchyNode= GetHierarchyNode(scene.GetPointer(), 0);
  hierarchyNode->GetDisplayNode()->SetVisibility(0);
  bool res = true;
  res = TestColors(scene.GetPointer(), DEFAULT_MODEL_COLORS, DEFAULT_HIERARCHY_COLORS) && res;
  int newHierarchyVisibility[3] = {0, 1, 1};
  res = TestVisibility(scene.GetPointer(), DEFAULT_MODEL_VISIBILITY, newHierarchyVisibility) && res;

  return res;
}

//---------------------------------------------------------------------------
bool TestSetVisibilityLevel1()
{
  vtkNew<vtkMRMLScene> scene;
  PopulateScene(scene.GetPointer(), LEVEL_COUNT);

  // Color and visibility shouldn't be changed when collapsing hierarchy
  vtkMRMLDisplayableHierarchyNode* hierarchyNode= GetHierarchyNode(scene.GetPointer(), 1);
  hierarchyNode->GetDisplayNode()->SetVisibility(0);
  bool res = true;
  res = TestColors(scene.GetPointer(), DEFAULT_MODEL_COLORS, DEFAULT_HIERARCHY_COLORS) && res;
  int newHierarchyVisibility[3] = {1, 0, 1};
  res = TestVisibility(scene.GetPointer(), DEFAULT_MODEL_VISIBILITY, newHierarchyVisibility) && res;

  return res;
}

//---------------------------------------------------------------------------
bool TestCollapseLevel0()
{
  vtkNew<vtkMRMLScene> scene;
  PopulateScene(scene.GetPointer(), LEVEL_COUNT);

  // Color and visibility shouldn't be changed when collapsing hierarchy
  vtkMRMLDisplayableHierarchyNode* hierarchyNode= GetHierarchyNode(scene.GetPointer(), 0);
  hierarchyNode->SetExpanded(0);
  bool res = true;
  res = TestColors(scene.GetPointer(), DEFAULT_MODEL_COLORS, DEFAULT_HIERARCHY_COLORS) && res;
  res = TestVisibility(scene.GetPointer(), DEFAULT_MODEL_VISIBILITY, DEFAULT_HIERARCHY_VISIBILITY) && res;
  if (!res)
    {
    std::cout << "Color or visibility is wrong" << std::endl;
    }

  if (GetModelHierarchyNode(scene.GetPointer(), 0)->GetCollapsedParentNode() != nullptr ||
      GetModelHierarchyNode(scene.GetPointer(), 1)->GetCollapsedParentNode() != hierarchyNode ||
      GetModelHierarchyNode(scene.GetPointer(), 2)->GetCollapsedParentNode() != hierarchyNode)
    {
    std::cout << "CollapsedParentNode is wrong (hierarchy node at level 0 collapsed):" << std::endl
              << GetModelHierarchyNode(scene.GetPointer(), 0)->GetCollapsedParentNode() << " (expected 0), "
              << GetModelHierarchyNode(scene.GetPointer(), 1)->GetCollapsedParentNode() << " (expected " << hierarchyNode << "), "
              << GetModelHierarchyNode(scene.GetPointer(), 2)->GetCollapsedParentNode() << " (expected  " << hierarchyNode << ")"
              << std::endl;
    res = false;
    }

  return res;
}

//---------------------------------------------------------------------------
bool TestCollapseLevel1()
{
  vtkNew<vtkMRMLScene> scene;
  PopulateScene(scene.GetPointer(), LEVEL_COUNT);

  // Color and visibility shouldn't be changed when collapsing hierarchy
  vtkMRMLDisplayableHierarchyNode* hierarchyNode= GetHierarchyNode(scene.GetPointer(), 1);
  hierarchyNode->SetExpanded(0);
  bool res = true;
  res = TestColors(scene.GetPointer(), DEFAULT_MODEL_COLORS, DEFAULT_HIERARCHY_COLORS) && res;
  res = TestVisibility(scene.GetPointer(), DEFAULT_MODEL_VISIBILITY, DEFAULT_HIERARCHY_VISIBILITY) && res;
  if (!res)
    {
    std::cout << "Color or visibility is wrong" << std::endl;
    }

  if (GetModelHierarchyNode(scene.GetPointer(), 0)->GetCollapsedParentNode() != nullptr ||
      GetModelHierarchyNode(scene.GetPointer(), 1)->GetCollapsedParentNode() != nullptr ||
      GetModelHierarchyNode(scene.GetPointer(), 2)->GetCollapsedParentNode() != hierarchyNode)
    {
    std::cout << "CollapsedParentNode is wrong (hierarchyNode at level 1 collapsed):" << std::endl
              << GetModelHierarchyNode(scene.GetPointer(), 0)->GetCollapsedParentNode() << " (expected 0), "
              << GetModelHierarchyNode(scene.GetPointer(), 1)->GetCollapsedParentNode() << " (expected 0), "
              << GetModelHierarchyNode(scene.GetPointer(), 2)->GetCollapsedParentNode() << " (expected " << hierarchyNode << ")"
              << std::endl;
    res = false;
    }

  return res;
}

//---------------------------------------------------------------------------
bool TestCollapseLevels0And1()
{
  vtkNew<vtkMRMLScene> scene;
  PopulateScene(scene.GetPointer(), LEVEL_COUNT);

  // Color and visibility shouldn't be changed when collapsing hierarchy
  vtkMRMLDisplayableHierarchyNode* hierarchyNode= GetHierarchyNode(scene.GetPointer(), 0);
  hierarchyNode->SetExpanded(0);
  vtkMRMLDisplayableHierarchyNode* subHierarchyNode= GetHierarchyNode(scene.GetPointer(), 1);
  subHierarchyNode->SetExpanded(0);
  bool res = true;
  res = TestColors(scene.GetPointer(), DEFAULT_MODEL_COLORS, DEFAULT_HIERARCHY_COLORS) && res;
  res = TestVisibility(scene.GetPointer(), DEFAULT_MODEL_VISIBILITY, DEFAULT_HIERARCHY_VISIBILITY) && res;
  if (!res)
    {
    std::cout << "Color or visibility is wrong" << std::endl;
    }

  if (GetModelHierarchyNode(scene.GetPointer(), 0)->GetCollapsedParentNode() != nullptr ||
      GetModelHierarchyNode(scene.GetPointer(), 1)->GetCollapsedParentNode() != hierarchyNode ||
      GetModelHierarchyNode(scene.GetPointer(), 2)->GetCollapsedParentNode() != hierarchyNode)
    {
    std::cout << "CollapsedParentNode is wrong (hierarchies at levels 0 and 1 collapsed):" << std::endl
              << GetModelHierarchyNode(scene.GetPointer(), 0)->GetCollapsedParentNode() << " (expected 0), "
              << GetModelHierarchyNode(scene.GetPointer(), 1)->GetCollapsedParentNode() << " (expected " << hierarchyNode << "), "
              << GetModelHierarchyNode(scene.GetPointer(), 2)->GetCollapsedParentNode() << " (expected " << hierarchyNode << ")"
              << std::endl;
    res = false;
    }

  return res;
}

//---------------------------------------------------------------------------
bool TestColors(vtkMRMLScene* scene,
                double (*modelColors)[3],
                double (*hierarchyColors)[3])
{
  for (int level = 0; level < LEVEL_COUNT; ++level)
    {
    // Model
    vtkMRMLModelNode* modelNode = GetModelNode(scene, level);
    if (modelNode == nullptr)
      {
      return true;
      }
    vtkMRMLDisplayNode* modelDisplayNode = modelNode->GetDisplayNode();
    if (modelDisplayNode->GetColor()[0] != modelColors[level][0] ||
        modelDisplayNode->GetColor()[1] != modelColors[level][1] ||
        modelDisplayNode->GetColor()[2] != modelColors[level][2])
      {
      std::cout << "Wrong color for node \"" << modelDisplayNode->GetName()
                << "\" at level " << level << std::endl;
      return false;
      }
    vtkMRMLDisplayableHierarchyNode* hierarchyNode = GetHierarchyNode(scene, level);
    assert(hierarchyNode != nullptr);
    vtkMRMLDisplayNode* hierarchyDisplayNode = hierarchyNode->GetDisplayNode();
    if (hierarchyDisplayNode->GetColor()[0] != hierarchyColors[level][0] ||
        hierarchyDisplayNode->GetColor()[1] != hierarchyColors[level][1] ||
        hierarchyDisplayNode->GetColor()[2] != hierarchyColors[level][2])
      {
      std::cout << "Wrong color for node \"" << hierarchyDisplayNode->GetName()
                << "\" at level " << level << std::endl;
      return false;
      }
    }
  return true;
}

//---------------------------------------------------------------------------
bool TestVisibility(vtkMRMLScene* scene,
                    int* modelVisibility,
                    int* hierarchyVisibility)
{
  for (int level = 0; level < LEVEL_COUNT; ++level)
    {
    // Model
    vtkMRMLModelNode* modelNode = GetModelNode(scene, level);
    if (modelNode == nullptr)
      {
      return true;
      }
    vtkMRMLDisplayNode* modelDisplayNode = modelNode->GetDisplayNode();
    if (modelDisplayNode->GetVisibility() != modelVisibility[level])
      {
      std::cout << "Wrong visibility for node \"" << modelDisplayNode->GetName()
                << "\" at level " << level << std::endl;
      return false;
      }
    vtkMRMLDisplayableHierarchyNode* hierarchyNode = GetHierarchyNode(scene, level);
    assert(hierarchyNode != nullptr);
    vtkMRMLDisplayNode* hierarchyDisplayNode = hierarchyNode->GetDisplayNode();
    if (hierarchyDisplayNode->GetVisibility() != hierarchyVisibility[level])
      {
      std::cout << "Wrong visibility for node \"" << hierarchyDisplayNode->GetName()
                << "\" at level " << level << std::endl;
      return false;
      }
    }
  return true;
}

} // end of anonymous namespace
